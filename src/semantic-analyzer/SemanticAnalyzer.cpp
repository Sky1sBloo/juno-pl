module;
#include <format>
#include <string>
#include <utility>
#include <variant>
module junopl.semantic;

/**
For visitors
 */
template <class... Ts> struct Overload : Ts... {
    using Ts::operator()...;
};

namespace JunoPL {
std::vector<Diagnostic> SemanticAnalyzer::analyze(const RootNode &root) {
    pushScope(); // global scope

    for (const auto &node : root.body) {
        std::visit(Overload{
                       [](const ImportNode &) {
                           // resolved by the file reader
                       },
                       [this](const EventNode &event) {
                           pushScope();
                           for (const auto &param : event.params) {
                               declare(Symbol{param, Symbol::Kind::VALUE, 0,
                                              event.loc});
                           }
                           visitBody(event.body);
                           popScope();
                       },
                       [this](const FunctionNode &func) {
                           declare(Symbol{func.identifier, Symbol::Kind::VALUE,
                                          func.params.size(), func.loc});
                           pushScope();
                           for (const auto &param : func.params) {
                               declare(Symbol{param, Symbol::Kind::VALUE, 0,
                                              func.loc});
                           }
                           visitBody(func.body);
                           popScope();
                       },
                       [this](const VarDeclaration &varDecl) {
                           if (varDecl.value) {
                               visitExpression(*varDecl.value);
                           }
                           declare(Symbol{varDecl.identifier,
                                          Symbol::Kind::VALUE, 0, varDecl.loc});
                       },
                       [this](const CustomExpression &expr) {
                           declare(Symbol{expr.identifier, Symbol::Kind::VALUE,
                                          expr.params.size(), expr.loc});
                           pushScope();
                           for (const auto &param : expr.params) {
                               declare(Symbol{param, Symbol::Kind::VALUE, 0,
                                              expr.loc});
                           }
                           visitExpression(*expr.expression);
                           popScope();
                       },
                       [this](const ListDeclaration &listDecl) {
                           for (const auto &value : listDecl.values) {
                               if (value) {
                                   visitExpression(*value);
                               }
                           }
                           declare(Symbol{listDecl.identifier,
                                          Symbol::Kind::LIST, 0, listDecl.loc});
                       },
                   },
                   node);
    }

    popScope(); // global scope
    return std::move(mDiagnostics);
}

void SemanticAnalyzer::visitBody(const Body &body) {
    pushScope();
    for (const auto &statement : body.body) {
        if (statement) {
            visitStatement(*statement);
        }
    }
    popScope();
}

void SemanticAnalyzer::visitExpression(const Expression &expr) {
    std::visit(
        Overload{
            [this](const Value &value) {
                if (value.kind == Value::Kind::Identifier) {
                    expect(value.value, value.loc);
                }
            },
            [this](const CallExpression &call) {
                if (!call.qualifier.empty()) {
                    expectQualified(call.qualifier, call.identifier,
                                    call.params.size(),
                                    JunoSTL::Operations::EXPRESSION, call.loc);
                } else {
                    expect(call.identifier, call.loc);
                }
                for (const auto &param : call.params) {
                    if (param) {
                        visitExpression(*param);
                    }
                }
            },
            [this](const ListIndex &listIndex) {
                expect(listIndex.identifier, listIndex.loc);
                visitExpression(*listIndex.index);
            },
            [this](const BinaryOp &binOp) {
                visitExpression(*binOp.lhs);
                visitExpression(*binOp.rhs);
            },
            [this](const UnaryOp &unOp) { visitExpression(*unOp.value); },
            [this](const TernaryOp &ternary) {
                visitExpression(*ternary.condition);
                visitExpression(*ternary.valueIfTrue);
                visitExpression(*ternary.valueIfFalse);
            },
            [](const auto &) {},
        },
        expr.value);
}

void SemanticAnalyzer::visitStatement(const Statements &statement) {
    std::visit(
        Overload{[this](const Body &body) { visitBody(body); },
                 [this](const VarDeclaration &varDecl) {
                     if (varDecl.value) {
                         visitExpression(*varDecl.value);
                     }
                     declare(Symbol{varDecl.identifier, Symbol::Kind::VALUE, 0,
                                    varDecl.loc});
                 },
                 [this](const VarAssignment &varAssign) {
                     expect(varAssign.identifier, varAssign.loc);
                     if (varAssign.indexExpr) {
                         visitExpression(*varAssign.indexExpr);
                     }
                     if (varAssign.value) {
                         visitExpression(*varAssign.value);
                     }
                 },
                 [this](const ListDeclaration &listDecl) {
                     for (const auto &value : listDecl.values) {
                         if (value) {
                             visitExpression(*value);
                         }
                     }
                     declare(Symbol{listDecl.identifier, Symbol::Kind::LIST, 0,
                                    listDecl.loc});
                 },
                 [this](const PerformInstruction &performInstr) {
                     if (!performInstr.qualifier.empty()) {
                         expectQualified(performInstr.qualifier,
                                         performInstr.identifier,
                                         performInstr.params.size(),
                                         JunoSTL::Operations::INSTRUCTION,
                                         performInstr.loc);
                     } else {
                         expect(performInstr.identifier, performInstr.loc);
                     }
                     for (const auto &param : performInstr.params) {
                         if (param) {
                             visitExpression(*param);
                         }
                     }
                 },
                 [this](const BreakStatement &) {},
                 [this](const ConditionalStatement &cond) {
                     visitExpression(*cond.ifStatement.condition);
                     visitBody(cond.ifStatement.body);
                     for (const auto &elif : cond.elifStatements) {
                         visitExpression(*elif.condition);
                         visitBody(elif.body);
                     }
                     visitBody(cond.elseStatement);
                 },
                 [this](const EmitEvent &emitEv) {
                     // avoided checking ident to allow other crafts to use
                     // pure vizzy
                     for (const auto &par : emitEv.params) {
                         if (par) {
                             visitExpression(*par);
                         }
                     }
                 },
                 [this](const RepeatLoop &repeatLoop) {
                     visitExpression(*repeatLoop.repeatNumber);
                     visitBody(repeatLoop.body);
                 },
                 [this](const WhileLoop &whileLoop) {
                     visitExpression(*whileLoop.condition);
                     visitBody(whileLoop.body);
                 },
                 [this](const ForLoop &forLoop) {
                     pushScope();
                     declare(Symbol{forLoop.iterator, Symbol::Kind::VALUE, 0,
                                    forLoop.loc});
                     visitExpression(*forLoop.start);
                     visitExpression(*forLoop.end);
                     visitExpression(*forLoop.by);
                     for (const auto &statement : forLoop.body.body) {
                         if (statement) {
                             visitStatement(*statement);
                         }
                     }
                     popScope();
                 }},
        statement.statement);
}

void SemanticAnalyzer::pushScope() { mScopes.emplace_back(); }

void SemanticAnalyzer::popScope() { mScopes.pop_back(); }

std::optional<Symbol> SemanticAnalyzer::lookup(const std::string &name) const {
    for (auto scope = mScopes.rbegin(); scope != mScopes.rend(); ++scope) {
        auto found = scope->symbols.find(name);
        if (found != scope->symbols.end()) {
            return found->second;
        }
    }
    return std::nullopt;
}

bool SemanticAnalyzer::expect(const std::string &name,
                              const SourceLocation &source) {
    if (auto symbol = lookup(name); !symbol.has_value()) {
        mDiagnostics.emplace_back(std::format("'{}' is not defined", name),
                                  source);
        return false;
    }
    return true;
}

bool SemanticAnalyzer::expectQualified(const std::string &qualifier,
                                       const std::string &identifier,
                                       std::size_t argCount,
                                       JunoSTL::Operations op,
                                       const SourceLocation &source) {
    if (auto func = JunoSTL::find(qualifier, identifier, op);
        func.has_value()) {
        if (!func->accepts(argCount)) {
            mDiagnostics.emplace_back(
                std::format("'{}.{}' expects {}, got {}", qualifier, identifier,
                            func->describeArities(), argCount),
                source);
            return false;
        }
        return true;
    }
    mDiagnostics.emplace_back(
        std::format("'{}.{}' is not defined in the standard library", qualifier,
                    identifier),
        source);
    return false;
}

bool SemanticAnalyzer::declare(Symbol symbol) {
    auto &symbols = mScopes.back().symbols;
    if (auto existing = symbols.find(symbol.name); existing != symbols.end()) {
        mDiagnostics.emplace_back(
            std::format("'{}' is already declared in this scope", symbol.name),
            symbol.declaredAt);
        return false;
    }
    symbols.emplace(symbol.name, std::move(symbol));
    return true;
}

}

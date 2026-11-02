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
    return {};
}

/**
Creates a new lexical scope for the body, analyzes every statement in
it, then discards the scope. This one function handles all three
placements of a Body in the AST: function bodies, event bodies, and bare
nested-block statements (the Body alternative of Statements), so nested
blocks get scoping for free.

Note for the future ForLoop analysis: the iterator must be visible
inside the loop body, so the for-loop case should pushScope(), declare
the iterator, and walk the body's statements directly instead of going
through this function (which would seal the iterator off in an outer
scope).
 */
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
                                    call.params.size(), call.loc);
                } else {
                    expect(call.identifier, call.loc);
                }
                for (const auto &param : call.params) {
                    if (param) {
                        visitExpression(*param);
                    }
                }
            },
            [this](const ListOp &listOp) {
                expect(listOp.identifier, listOp.loc);
                for (const auto &param : listOp.params) {
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
            [this](const UnaryOp &unOp) {
                visitExpression(*unOp.value);
            },
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
    std::visit(Overload{
                   [this](const Body &body) { visitBody(body); },
                   [this](const VarDeclaration &varDecl) {
                       declare(Symbol{varDecl.identifier, Symbol::Kind::VALUE,
                                      0, varDecl.loc});
                   },
                   [this](const VarAssignment &varAssign) {
                       expect(varAssign.identifier, varAssign.loc);
                   },
                   [this](const ListDeclaration &listDecl) {
                       declare(Symbol{listDecl.identifier, Symbol::Kind::LIST,
                                      0, listDecl.loc});
                   },
                   [this](const PerformInstruction &performInstr) {
                       if (!performInstr.qualifier.empty()) {
                           expectQualified(
                               performInstr.qualifier, performInstr.identifier,
                               performInstr.params.size(), performInstr.loc);
                       } else {
                           expect(performInstr.identifier, performInstr.loc);
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
                   [this](const EmitEvent &) {},
                   [this](const RepeatLoop &) {},
                   [this](const WhileLoop &) {},
                   [this](const ForLoop &) {},
                   [this](const ListOp &) {},
               },
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
                                       const SourceLocation &source) {
    if (auto func = JunoSTL::find(qualifier, identifier); func.has_value()) {
        if (func->arity != argCount) {
            mDiagnostics.emplace_back(
                std::format("'{}.{}' expects {} arguments, got {}", qualifier,
                            identifier, func->arity, argCount),
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

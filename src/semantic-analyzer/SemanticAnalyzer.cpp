module;
#include <format>
#include <string>
#include <utility>
#include <variant>
module junopl.semantic;

/**
For visitors
 */
template <class ...Ts>
struct Overload : Ts... { using Ts::operator()...; };

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

void SemanticAnalyzer::visitStatement(const Statements &statement) {
    std::visit(
        Overload{
            [this](const Body &body) { visitBody(body); },
            [this](const VarDeclaration &) {},
            [this](const VarAssignment &) {},
            [this](const ListDeclaration &) {},
            [this](const PerformInstruction &) {},
            [this](const BreakStatement &) {},
            [this](const ConditionalStatement &) {},
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

/**
Resolves a name starting from the innermost scope outward. Returns
nullptr when the name is not declared anywhere, which callers turn into
an "undefined variable" diagnostic.
 */
Symbol *SemanticAnalyzer::lookup(const std::string &name) {
    for (auto scope = mScopes.rbegin(); scope != mScopes.rend(); ++scope) {
        auto found = scope->symbols.find(name);
        if (found != scope->symbols.end()) {
            return &found->second;
        }
    }
    return nullptr;
}

/**
Introduces a name in the current (innermost) scope only. Shadowing an
outer declaration is fine; redeclaring in the same scope is an error.
Returns false and records a diagnostic when the name is taken.
 */
bool SemanticAnalyzer::declare(Symbol symbol) {
    auto &symbols = mScopes.back().symbols;
    if (auto existing = symbols.find(symbol.name); existing != symbols.end()) {
        mDiagnostics.push_back(Diagnostic{
            std::format("'{}' is already declared in this scope", symbol.name),
            symbol.declaredAt});
        return false;
    }
    symbols.emplace(symbol.name, std::move(symbol));
    return true;
}

}

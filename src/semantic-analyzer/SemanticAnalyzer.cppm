module;
#include <optional>
#include <string>
#include <vector>
export module junopl.semantic;

import junopl.parser.nodes;
import junopl.parser.nodes.statements;
import junopl.parser.nodes.expressions;
import junopl.semantic.symbols;
import junopl.semantic.diagnostic;
import junopl.common.location;
import junopl.stl;
namespace JunoPL {
export class SemanticAnalyzer {
  public:
    /**
Runs both passes over the program and returns every diagnostic found.
Invariant: analyze() pushes the global scope before anything else, so
declare() and lookup() are only valid while at least that scope exists.
     */
    std::vector<Diagnostic> analyze(const RootNode &root);

  private:
    // Recursive walk
    void visitBody(const Body &body);
    void visitStatement(const Statements &stmt);
    void visitExpression(const Expression &expr);

    // Scope stack
    void pushScope();
    void popScope();

    // For handling diagnostics for callers
    std::optional<Symbol> lookup(const std::string &name) const;

    // For handling diagnostics automatically
    bool expect(const std::string &name, const SourceLocation &source);
    // For stl functions
    bool expectQualified(const std::string &qualifier,
                         const std::string &identifier, std::size_t argCount,
                         JunoSTL::Operations op, const SourceLocation &source);
    bool declare(Symbol symbol);

    std::vector<Scope> mScopes;
    std::vector<Diagnostic> mDiagnostics;
    int mLoopDepth{0};
};
}

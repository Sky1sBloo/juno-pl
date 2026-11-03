module;
#include <cstddef>
#include <expected>
#include <filesystem>
#include <functional>
#include <map>
#include <pugixml.hpp>
#include <set>
#include <string>
#include <vector>
export module junopl.compiler;

import junopl.parser.nodes;
import junopl.parser.nodes.statements;
import junopl.parser.nodes.expressions;
import junopl.filehandler;

namespace JunoPL {
export class Compiler {
  public:
    /** Returns the compiled Vizzy XML as a string. */
    std::string compileAST(const RootNode &root);

    /** Writes the compiled XML string to a file. */
    std::expected<void, FileHandlerError>
    writeToFile(const std::filesystem::path &path,
                const std::string &xml);

    /**
    Traverses the AST and collects every variable declaration so the codegen
    can emit each one as a Vizzy global (Vizzy has no local variables).

    The map is keyed by scope path + name: just "name" for a top level
    variable, "path.name" for a scoped one, e.g. "main.x", the body of a
    while at statement 1 as "main.1.y", or a for loop iterator nested in an
    if as "main.2.if.0.i". Components are joined with '.' because an
    identifier can never contain one, so a scoped key can never collide with
    a global's plain key and no '/' can end up in anything derived from it.
    Only the value is ever emitted as a variable name.

    The value is the name to emit. Top level variables keep their own name;
    scoped ones become "name_hash", e.g. "i_9db4c1a2", so the original name
    stays readable as a prefix while the hash of the key keeps same named
    variables from different scopes - for loop iterators included - apart.
    A generated name that is already taken gains a counter suffix.
     */
    std::map<std::string, std::string> getVariables(const RootNode &root);

  private:
    /**
    Stores expressions, events, and instructions
     */
    void writeHeader();
    void loadFunctions();

    /** Scope-tracking traversal used by getVariables. */
    void collectBody(const Body &body);
    void collectStatement(const Statements &stmt, std::size_t index);

    /** pugixml-emitting traversal used by compileAST. */
    void emitBody(const Body &body, pugi::xml_node parent,
                  std::function<std::string()> nextId);
    void emitStatement(const Statements &stmt, std::size_t index,
                       pugi::xml_node parent,
                       std::function<std::string()> nextId);
    void emitExpression(const Expression &expr, pugi::xml_node parent);
    void emitCondition(const Expression &expr, pugi::xml_node parent);

    /** Key/value table built by getVariables. */
    std::map<std::string, std::string> mVariables;
    /** Components of the scope being visited; empty at top level. */
    std::vector<std::string> mScopePath;
    /** Every name already claimed, so generated names stay unique. */
    std::set<std::string> mUsedNames;

    /**
    Claims the name of each top level variable, which is already global, so
    no generated name can take it.
     */
    void reserveGlobals(const RootNode &root);

    /** Registers a variable declared in the scope currently being visited. */
    void declareScoped(const std::string &name);

    /** "name" plus the hash of key, made unique against mUsedNames. */
    std::string makeUnique(const std::string &name, const std::string &key);

    void visitBody(const Body &body);
    /**
    index is the position of the statement in its body. It becomes the path
    component of any scope the statement opens, which is what keeps two
    sibling blocks that declare the same name apart.
     */
    void visitStatement(const Statements &stmt, std::size_t index);
    void visitExpression(const Expression &expr);
};
}

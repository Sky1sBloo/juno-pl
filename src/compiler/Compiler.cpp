module;
#include <cstddef>
#include <cstdint>
#include <format>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <variant>
#include <vector>
module junopl.compiler;

/**
For visitors
 */
template <class... Ts> struct Overload : Ts... {
    using Ts::operator()...;
};

namespace JunoPL {
void Compiler::compileAST(const RootNode &root) {}

std::map<std::string, std::string>
Compiler::getVariables(const RootNode &root) {
    mVariables.clear();
    mScopePath.clear();
    mUsedNames.clear();

    reserveGlobals(root);

    for (const auto &node : root.body) {
        std::visit(Overload{
                       [this](const EventNode &event) {
                           mScopePath.push_back(event.identifier);
                           visitBody(event.body);
                           mScopePath.pop_back();
                       },
                       [this](const FunctionNode &function) {
                           mScopePath.push_back(function.identifier);
                           visitBody(function.body);
                           mScopePath.pop_back();
                       },
                       [](const auto &) {
                           // imports and top level variables, already reserved;
                           // expressions hold no declarations
                       },
                   },
                   node);
    }

    return std::move(mVariables);
}

void Compiler::reserveGlobals(const RootNode &root) {
    for (const auto &node : root.body) {
        std::visit(Overload{
                       [this](const VarDeclaration &varDecl) {
                           mVariables.emplace(varDecl.identifier,
                                              varDecl.identifier);
                           mUsedNames.insert(varDecl.identifier);
                       },
                       [this](const ListDeclaration &listDecl) {
                           mVariables.emplace(listDecl.identifier,
                                              listDecl.identifier);
                           mUsedNames.insert(listDecl.identifier);
                       },
                       [](const auto &) {},
                   },
                   node);
    }
}

void Compiler::declareScoped(const std::string &name) {
    std::string key;
    for (const auto &component : mScopePath) {
        key += component;
        key += '.';
    }
    key += name;

    mVariables.emplace(key, makeUnique(name, key));
}

std::string Compiler::makeUnique(const std::string &name,
                                 const std::string &key) {
    // FNV-1a rather than std::hash: the emitted names have to stay the same
    // across builds and standard libraries.
    std::uint32_t hash = 2166136261u;
    for (unsigned char byte : key) {
        hash ^= byte;
        hash *= 16777619u;
    }

    auto candidate = std::format("{}_{:08x}", name, hash);
    for (unsigned suffix = 1; !mUsedNames.insert(candidate).second; ++suffix) {
        candidate = std::format("{}_{:08x}_{}", name, hash, suffix);
    }
    return candidate;
}

void Compiler::visitBody(const Body &body) {
    for (std::size_t index = 0; index < body.body.size(); ++index) {
        if (body.body[index]) {
            visitStatement(*body.body[index], index);
        }
    }
}

void Compiler::visitStatement(const Statements &statement, std::size_t index) {
    // Path component of every scope this statement opens.
    const auto scope = std::to_string(index);

    std::visit(Overload{
                   [this, scope](const Body &body) {
                       mScopePath.push_back(scope);
                       visitBody(body);
                       mScopePath.pop_back();
                   },
                   [this](const VarDeclaration &varDecl) {
                       declareScoped(varDecl.identifier);
                   },
                   [](const VarAssignment &) {
                       // assigns to an existing variable, declares nothing
                   },
                   [this](const ListDeclaration &listDecl) {
                       declareScoped(listDecl.identifier);
                   },
                   [](const PerformInstruction &) {},
                   [](const BreakStatement &) {},
                   [this, scope](const ConditionalStatement &conditional) {
                       mScopePath.push_back(scope + ".if");
                       visitBody(conditional.ifStatement.body);
                       mScopePath.pop_back();

                       for (std::size_t i = 0;
                            i < conditional.elifStatements.size(); ++i) {
                           mScopePath.push_back(scope + ".elif" +
                                                std::to_string(i));
                           visitBody(conditional.elifStatements[i].body);
                           mScopePath.pop_back();
                       }

                       mScopePath.push_back(scope + ".else");
                       visitBody(conditional.elseStatement);
                       mScopePath.pop_back();
                   },
                   [](const EmitEvent &) {},
                   [this, scope](const RepeatLoop &repeatLoop) {
                       mScopePath.push_back(scope);
                       visitBody(repeatLoop.body);
                       mScopePath.pop_back();
                   },
                   [this, scope](const WhileLoop &whileLoop) {
                       mScopePath.push_back(scope);
                       visitBody(whileLoop.body);
                       mScopePath.pop_back();
                   },
                   [this, scope](const ForLoop &forLoop) {
                       // The iterator and the body share one scope, matching
                       // the semantic analyzer, so a body variable named like
                       // the iterator is a redeclaration rather than a shadow.
                       mScopePath.push_back(scope);
                       declareScoped(forLoop.iterator);
                       visitBody(forLoop.body);
                       mScopePath.pop_back();
                   },
               },
               statement.statement);
}
}

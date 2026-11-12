module;
#include <cstddef>
#include <format>
#include <map>
#include <set>
#include <string>
#include <variant>
#include <vector>
#include "CodegenSupport.hpp"
module junopl.compiler;
import junopl.parser.nodes;
import junopl.parser.nodes.expressions;
import junopl.parser.nodes.statements;

namespace JunoPL {

std::map<std::string, std::string>
Compiler::getVariables(const RootNode &root) {
    mVariables.clear();
    mVariableOrder.clear();
    mLists.clear();
    mScopePath.clear();
    mUsedNames.clear();

    reserveGlobals(root);

    for (const auto &node : root.body) {
        std::visit(Overload{
                       [this](const EventNode &event) {
                           mScopePath.push_back(event.identifier);
                           collectBody(event.body);
                           mScopePath.pop_back();
                       },
                       [this](const FunctionNode &function) {
                           mScopePath.push_back(function.identifier);
                           collectBody(function.body);
                           mScopePath.pop_back();
                       },
                       [](const auto &) {
                           // imports and top level variables, already reserved;
                           // expressions hold no declarations
                       },
                   },
                   node);
    }

    return mVariables;
}

void Compiler::reserveGlobals(const RootNode &root) {
    for (const auto &node : root.body) {
        std::visit(Overload{
                       [this](const VarDeclaration &varDecl) {
                           mUsedNames.insert(varDecl.identifier);
                           if (mVariables
                                   .emplace(varDecl.identifier,
                                            varDecl.identifier)
                                   .second) {
                               mVariableOrder.push_back(
                                   varDecl.identifier);
                           }
                       },
                       [this](const ListDeclaration &listDecl) {
                           mUsedNames.insert(listDecl.identifier);
                           if (mVariables
                                   .emplace(listDecl.identifier,
                                            listDecl.identifier)
                                   .second) {
                               mVariableOrder.push_back(
                                   listDecl.identifier);
                               mLists.insert(listDecl.identifier);
                           }
                       },
                       [](const auto &) {},
                   },
                   node);
    }
}

void Compiler::declareScoped(const std::string &name, bool isList) {
    std::string key;
    for (const auto &component : mScopePath) {
        key += component;
        key += '.';
    }
    key += name;

    auto emitted = makeUnique(name, key);
    if (mVariables.emplace(key, emitted).second) {
        mVariableOrder.push_back(emitted);
        if (isList)
            mLists.insert(emitted);
    }
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

std::string Compiler::resolveVariableName(const std::string &name) {
    // Try from innermost scope outward, keeping the variable name:
    // "start.1.if.a" → "start.1.a" → "start.a" → "a"
    for (std::size_t i = mScopePath.size(); i > 0; --i) {
        std::string key;
        for (std::size_t j = 0; j < i; ++j) {
            key += mScopePath[j];
            key += '.';
        }
        key += name;
        auto it = mVariables.find(key);
        if (it != mVariables.end()) {
            return it->second;
        }
    }
    // Fall back to raw name (global or not yet registered)
    auto it = mVariables.find(name);
    if (it != mVariables.end()) {
        return it->second;
    }
    return name;
}

void Compiler::collectBody(const Body &body) {
    for (std::size_t index = 0; index < body.body.size(); ++index) {
        if (body.body[index]) {
            collectStatement(*body.body[index], index);
        }
    }
}

void Compiler::collectStatement(const Statements &statement,
                                std::size_t index) {
    const auto scope = std::to_string(index);

    std::visit(Overload{
                   [this, scope](const Body &body) {
                       mScopePath.push_back(scope);
                       collectBody(body);
                       mScopePath.pop_back();
                   },
                   [this](const VarDeclaration &varDecl) {
                       declareScoped(varDecl.identifier, false);
                   },
                   [](const VarAssignment &) {
                       // assigns to an existing variable, declares nothing
                   },
                   [this](const ListDeclaration &listDecl) {
                       declareScoped(listDecl.identifier, true);
                   },
                   [](const DoInstruction &) {},
                   [](const BreakStatement &) {},
                   [this, scope](const ConditionalStatement &conditional) {
                       mScopePath.push_back(scope + ".if");
                       collectBody(conditional.ifStatement.body);
                       mScopePath.pop_back();

                       for (std::size_t i = 0;
                            i < conditional.elifStatements.size(); ++i) {
                           mScopePath.push_back(scope + ".elif" +
                                                std::to_string(i));
                           collectBody(conditional.elifStatements[i].body);
                           mScopePath.pop_back();
                       }

                       mScopePath.push_back(scope + ".else");
                       collectBody(conditional.elseStatement);
                       mScopePath.pop_back();
                   },
                   [](const EmitEvent &) {},
                   [this, scope](const RepeatLoop &repeatLoop) {
                       mScopePath.push_back(scope);
                       collectBody(repeatLoop.body);
                       mScopePath.pop_back();
                   },
                   [this, scope](const WhileLoop &whileLoop) {
                       mScopePath.push_back(scope);
                       collectBody(whileLoop.body);
                       mScopePath.pop_back();
                   },
                   [this, scope](const ForLoop &forLoop) {
                       // The iterator and the body share one scope, matching
                       // the semantic analyzer, so a body variable named like
                       // the iterator is a redeclaration rather than a shadow.
                       mScopePath.push_back(scope);
                       declareScoped(forLoop.iterator, false);
                       collectBody(forLoop.body);
                       mScopePath.pop_back();
                   },
               },
                statement.statement);
    }
} // namespace JunoPL

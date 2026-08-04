module;
#include <string>
#include <variant>
export module junopl.parser.nodes.statements;

namespace JunoPL {
export struct VarDeclaration {
    std::string identifier;
    // Value Expression
};
export struct ListDeclaration {
    std::string identifier;
    // List Value expression
};

export using Statements = std::variant<VarDeclaration, ListDeclaration>;
}
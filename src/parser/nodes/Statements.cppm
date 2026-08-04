module;
#include <memory>
#include <string>
#include <variant>
#include <vector>
export module junopl.parser.nodes.statements;
import junopl.parser.nodes.expressions;

namespace JunoPL {
export struct Statements;
using StatementsHandle = std::unique_ptr<Statements>;
export struct VarDeclaration {
    std::string identifier;
    ExpressionHandle value;
};
export struct ListDeclaration {
    std::string identifier;
    std::vector<ExpressionHandle> values;
};

export struct PerformInstruction {
    std::string qualifier;
    std::string identifier;
    std::vector<ExpressionHandle> params;
};

export struct IfStatement {
    ExpressionHandle condition;
    std::vector<StatementsHandle> body;
};

export struct ElifStatement {
    ExpressionHandle condition;
    std::vector<StatementsHandle> body;
};

export struct IfChain {
    IfStatement ifStatement;
    std::vector<std::unique_ptr<ElifStatement>> elifStatements;
    std::vector<ExpressionHandle> elseStatement;
};

export struct Statements {
    std::variant<VarDeclaration, ListDeclaration, IfChain> statement;
};

}
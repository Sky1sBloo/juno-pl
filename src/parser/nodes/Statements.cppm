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
export struct Body {
    std::vector<ExpressionHandle> body;
};

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
    std::unique_ptr<Body> body;
};

export struct ElifStatement {
    ExpressionHandle condition;
    std::unique_ptr<Body> body;
};

export struct ConditionalStatement {
    IfStatement ifStatement;
    std::vector<std::unique_ptr<ElifStatement>> elifStatements;
    std::unique_ptr<Body> elseStatement;
};

export struct EmitEvent {
    enum Options { TO_SCRIPT, TO_CRAFT, TO_NEARBY_CRAFTS };
    std::string identifier;
    std::vector<ExpressionHandle> params;
    Options option;
};

export struct RepeatLoop {
    ExpressionHandle repeatNumber;
    std::vector<ExpressionHandle> body;
};

export struct WhileLoop {
    ExpressionHandle condition;
    std::unique_ptr<Body> body;
};

export struct Statements {
    std::variant<VarDeclaration, ListDeclaration, ConditionalStatement>
        statement;
};

}
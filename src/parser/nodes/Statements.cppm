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
    std::vector<StatementsHandle> body;
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
    Body body;
};

export struct ElifStatement {
    ExpressionHandle condition;
    Body body;
};

export struct ConditionalStatement {
    IfStatement ifStatement;
    std::vector<ElifStatement> elifStatements;
    Body elseStatement;
};

export struct EmitEvent {
    enum Options { TO_SCRIPT, TO_CRAFT, TO_NEARBY_CRAFTS };
    std::string identifier;
    std::vector<ExpressionHandle> params;
    Options option;
};

export struct RepeatLoop {
    ExpressionHandle repeatNumber;
    Body body;
};

export struct WhileLoop {
    ExpressionHandle condition;
    Body body;
};

export struct ForLoop {
    ExpressionHandle start;
    ExpressionHandle end;
    ExpressionHandle by;
    Body body;
};

export struct Statements {
    std::variant<Body, VarDeclaration, ListDeclaration, PerformInstruction,
                 ConditionalStatement, EmitEvent, RepeatLoop, WhileLoop,
                 ForLoop>
        statement;
};

}
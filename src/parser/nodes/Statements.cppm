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

export struct VarAssignment {
    std::string identifier;
    ExpressionHandle indexExpr;
    ExpressionHandle value;
    enum Op { ASSIGN, PLUS_ASSIGN, MINUS_ASSIGN, MULT_ASSIGN, DIVIDE_ASSIGN } op;
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

export struct BreakStatement {};

export struct IfStatement {
    ExpressionHandle condition;
    Body body;
};

export struct ConditionalStatement {
    IfStatement ifStatement;
    std::vector<IfStatement> elifStatements;
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
    std::string iterator;
    ExpressionHandle start;
    ExpressionHandle end;
    ExpressionHandle by;
    Body body;
};

export struct Statements {
    std::variant<Body, VarDeclaration, VarAssignment, ListDeclaration,
                 PerformInstruction, BreakStatement, ConditionalStatement,
                 EmitEvent, RepeatLoop, WhileLoop, ForLoop, ListOp>
        statement;
};

}

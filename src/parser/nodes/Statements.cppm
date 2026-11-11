module;
#include <memory>
#include <string>
#include <variant>
#include <vector>
export module junopl.parser.nodes.statements;
import junopl.common.location;
import junopl.parser.nodes.expressions;

namespace JunoPL {
export struct Statements;
using StatementsHandle = std::unique_ptr<Statements>;
export struct Body {
    std::vector<StatementsHandle> body;
    SourceLocation loc;
};

export struct VarDeclaration {
    std::string identifier;
    ExpressionHandle value;
    SourceLocation loc;
};

export struct VarAssignment {
    std::string identifier;
    ExpressionHandle indexExpr;
    ExpressionHandle value;
    enum Op { ASSIGN, PLUS_ASSIGN, MINUS_ASSIGN, MULT_ASSIGN, DIVIDE_ASSIGN } op;
    SourceLocation loc;
};

export struct ListDeclaration {
    std::string identifier;
    std::vector<ExpressionHandle> values;
    SourceLocation loc;
};

export struct DoInstruction {
    std::string qualifier;
    std::string identifier;
    std::vector<ExpressionHandle> params;
    bool isInline{false};
    SourceLocation loc;
};

export struct BreakStatement {
    SourceLocation loc;
};

export struct IfStatement {
    ExpressionHandle condition;
    Body body;
    SourceLocation loc;
};

export struct ConditionalStatement {
    IfStatement ifStatement;
    std::vector<IfStatement> elifStatements;
    Body elseStatement;
    SourceLocation loc;
};

export struct EmitEvent {
    enum Options { TO_SCRIPT, TO_CRAFT, TO_NEARBY_CRAFTS };
    std::string identifier;
    std::vector<ExpressionHandle> params;
    Options option;
    SourceLocation loc;
};

export struct RepeatLoop {
    ExpressionHandle repeatNumber;
    Body body;
    SourceLocation loc;
};

export struct WhileLoop {
    ExpressionHandle condition;
    Body body;
    SourceLocation loc;
};

export struct ForLoop {
    std::string iterator;
    ExpressionHandle start;
    ExpressionHandle end;
    ExpressionHandle by;
    Body body;
    SourceLocation loc;
};

export struct Statements {
    std::variant<Body, VarDeclaration, VarAssignment, ListDeclaration,
                 DoInstruction, BreakStatement, ConditionalStatement,
                 EmitEvent, RepeatLoop, WhileLoop, ForLoop>
        statement;

    SourceLocation loc() const {
        return std::visit(
            [](const auto &node) -> SourceLocation { return node.loc; },
            statement);
    }
};

} // namespace JunoPL

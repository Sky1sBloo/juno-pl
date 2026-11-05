module;
#include <memory>
#include <string>
#include <variant>
#include <vector>
export module junopl.parser.nodes.expressions;
import junopl.common.location;

namespace JunoPL {
export struct Expression;
export using ExpressionHandle = std::unique_ptr<Expression>;

export struct CallExpression {
    std::string qualifier;
    std::string identifier;
    std::vector<ExpressionHandle> params;
    SourceLocation loc;
};

export struct CustomExpression {
    std::string identifier;
    std::vector<std::string> params;
    ExpressionHandle expression;
    SourceLocation loc;
};

/**
A literal carries int, string or bool text, which the compiler writes the
way Vizzy stores it (number=, text=, bool=). Kind separates the three
cases that are otherwise textually identical in the source: a quoted
string such as "10000.99" has to stay text instead of becoming a number,
and an identifier reference such as 10000.99 without quotes is a name.
 */
export struct Value {
    std::string value;
    enum class Kind { Literal, String, Identifier } kind;
    SourceLocation loc;
};

export struct BinaryOp {
    enum class Operation {
        ADD,
        SUBTRUCT,
        MULTIPLY,
        DIVIDE,
        MOD,
        AND,
        OR,
        EQUAL,
        NOT_EQUAL,
        GREATER,
        LESS,
        GREATER_EQ,
        LESS_EQ
    };
    Operation operation;
    ExpressionHandle lhs;
    ExpressionHandle rhs;
    SourceLocation loc;
};

export struct UnaryOp {
    enum class Operation { NOT, NEGATE };
    Operation operation;
    ExpressionHandle value;
    SourceLocation loc;
};

export struct ListIndex {
    std::string identifier;
    ExpressionHandle index;
    SourceLocation loc;
};

export struct TernaryOp {
    ExpressionHandle condition;
    ExpressionHandle valueIfTrue;
    ExpressionHandle valueIfFalse;
    SourceLocation loc;
};

export struct Expression {
    std::variant<CallExpression, Value, BinaryOp, UnaryOp, ListIndex,
                 TernaryOp>
        value;

    SourceLocation loc() const {
        return std::visit(
            [](const auto &node) -> SourceLocation { return node.loc; },
            value);
    }
};

} // namespace JunoPL

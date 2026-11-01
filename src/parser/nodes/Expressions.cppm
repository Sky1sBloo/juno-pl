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
Can be int, string, bool as Vizzy doesn't separate them
 */
export struct Value {
    std::string value;
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

export struct ListOp {
    std::string identifier;
    std::string action;
    std::vector<ExpressionHandle> params;
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
    std::variant<CallExpression, Value, BinaryOp, UnaryOp, ListOp, ListIndex,
                 TernaryOp>
        value;

    SourceLocation loc() const {
        return std::visit(
            [](const auto &node) -> SourceLocation { return node.loc; },
            value);
    }
};

} // namespace JunoPL

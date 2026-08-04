module;
#include <memory>
#include <string>
#include <variant>
#include <vector>
export module junopl.parser.nodes.expressions;

namespace JunoPL {
export struct Expression;
export using ExpressionHandle = std::unique_ptr<Expression>;

export struct CallExpression {
    std::string qualifier;
    std::string identifier;
    std::vector<ExpressionHandle> params;
};

/**
Can be int, string, bool as Vizzy doesn't separate them
 */
export struct Value {
    std::string value;
};

export struct BinaryOp {
    enum Operation { ADD, SUBTRUCT, MULTIPLY, DIVIDE, MOD, AND, OR };
    Operation operation;
    ExpressionHandle lhs;
    ExpressionHandle rhs;
};

export struct UnaryOp {
    enum Operation { NOT };
    Operation operation;
    ExpressionHandle value;
};

export struct ListOp {
    std::string identifier;
    std::string action;
    std::vector<std::string> params;
};

export struct TernaryOp {
    ExpressionHandle condition;
    ExpressionHandle valueIfTrue;
    ExpressionHandle valueIfFalse;
};

export struct Expression {
    std::variant<CallExpression, Value> value;
};

}
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

export struct Expression {
    std::variant<CallExpression> value;
};

}
module;
#include <memory>
#include <optional>
#include <utility>
module junopl.parser;

namespace JunoPL {

namespace {
int binaryPrecedence(TokenType type) {
    switch (type) {
    case TokenType::K_OR:
        return 1;
    case TokenType::K_AND:
        return 2;
    case TokenType::OP_COMP_EQ:
    case TokenType::OP_COMP_NOT_EQ:
    case TokenType::OP_COMP_GREATER:
    case TokenType::OP_COMP_LESS:
    case TokenType::OP_COMP_GREATER_EQ:
    case TokenType::OP_COMP_LESS_EQ:
        return 3;
    case TokenType::OP_PLUS:
    case TokenType::OP_MINUS:
        return 4;
    case TokenType::OP_MULT:
    case TokenType::OP_DIVIDE:
    case TokenType::OP_MOD:
        return 5;
    default:
        return -1;
    }
}

std::optional<BinaryOp::Operation>
binaryOperationForToken(const Token &token) {
    switch (token.type) {
    case TokenType::OP_PLUS:
        return BinaryOp::Operation::ADD;
    case TokenType::OP_MINUS:
        return BinaryOp::Operation::SUBTRUCT;
    case TokenType::OP_MULT:
        return BinaryOp::Operation::MULTIPLY;
    case TokenType::OP_DIVIDE:
        return BinaryOp::Operation::DIVIDE;
    case TokenType::OP_MOD:
        return BinaryOp::Operation::MOD;
    case TokenType::K_AND:
        return BinaryOp::Operation::AND;
    case TokenType::K_OR:
        return BinaryOp::Operation::OR;
    case TokenType::OP_COMP_EQ:
        return BinaryOp::Operation::EQUAL;
    case TokenType::OP_COMP_NOT_EQ:
        return BinaryOp::Operation::NOT_EQUAL;
    case TokenType::OP_COMP_GREATER:
        return BinaryOp::Operation::GREATER;
    case TokenType::OP_COMP_LESS:
        return BinaryOp::Operation::LESS;
    case TokenType::OP_COMP_GREATER_EQ:
        return BinaryOp::Operation::GREATER_EQ;
    case TokenType::OP_COMP_LESS_EQ:
        return BinaryOp::Operation::LESS_EQ;
    default:
        return std::nullopt;
    }
}

std::optional<UnaryOp::Operation> unaryOperationForToken(const Token &token) {
    switch (token.type) {
    case TokenType::K_NOT:
        return UnaryOp::Operation::NOT;
    case TokenType::OP_MINUS:
        return UnaryOp::Operation::NEGATE;
    default:
        return std::nullopt;
    }
}
} // namespace

std::optional<ExpressionHandle> Parser::parsePrimaryExpression() {
    if (mTokens->empty()) {
        mErrors.emplace_back(ParserError::EmptyTokenList(TokenType::IDENT));
        return std::nullopt;
    }

    switch (mTokens->current().type) {
    case TokenType::NUM:
    case TokenType::STR:
    case TokenType::TRUE:
    case TokenType::FALSE:
    case TokenType::IDENT: {
        auto valueToken = mTokens->current();
        mTokens->advance();

        auto expression = std::make_unique<JunoPL::Expression>();
        expression->value = JunoPL::Value{valueToken.value};
        return expression;
    }
    case TokenType::OP_PAR_OP: {
        mTokens->advance();
        auto expression = parseTernaryExpression();
        if (!expression) {
            return std::nullopt;
        }

        if (!expectToken(TokenType::OP_PAR_CLO)) {
            return std::nullopt;
        }

        return expression;
    }
    default:
        mErrors.emplace_back(ParserError::UnexpectedToken(
            mTokens->current(), {TokenType::NUM, TokenType::STR, TokenType::TRUE,
                                TokenType::FALSE, TokenType::IDENT,
                                TokenType::OP_PAR_OP}));
        return std::nullopt;
    }
}

std::optional<ExpressionHandle> Parser::parseUnaryExpression() {
    if (mTokens->empty()) {
        mErrors.emplace_back(ParserError::EmptyTokenList(TokenType::IDENT));
        return std::nullopt;
    }

    switch (mTokens->current().type) {
    case TokenType::K_NOT:
    case TokenType::OP_MINUS: {
        auto operation = unaryOperationForToken(mTokens->current());
        if (!operation) {
            mErrors.emplace_back(ParserError::UnexpectedToken(
                mTokens->current(), {TokenType::K_NOT, TokenType::OP_MINUS}));
            return std::nullopt;
        }

        mTokens->advance();
        auto value = parseUnaryExpression();
        if (!value) {
            return std::nullopt;
        }

        auto expression = std::make_unique<JunoPL::Expression>();
        expression->value =
            JunoPL::UnaryOp{operation.value(), std::move(value.value())};
        return expression;
    }
    default:
        return parsePrimaryExpression();
    }
}

std::optional<ExpressionHandle>
Parser::parseBinaryExpression(int minimumPrecedence) {
    auto left = parseUnaryExpression();
    if (!left) {
        return std::nullopt;
    }

    while (!mTokens->empty()) {
        int precedence = binaryPrecedence(mTokens->current().type);
        if (precedence < minimumPrecedence) {
            break;
        }

        auto operation = binaryOperationForToken(mTokens->current());
        if (!operation) {
            mErrors.emplace_back(ParserError::UnexpectedToken(
                mTokens->current(),
                {TokenType::OP_PLUS,        TokenType::OP_MINUS,
                 TokenType::OP_MULT,        TokenType::OP_DIVIDE,
                 TokenType::OP_MOD,         TokenType::K_AND,
                 TokenType::K_OR,           TokenType::OP_COMP_EQ,
                 TokenType::OP_COMP_NOT_EQ, TokenType::OP_COMP_GREATER,
                 TokenType::OP_COMP_LESS,   TokenType::OP_COMP_GREATER_EQ,
                 TokenType::OP_COMP_LESS_EQ}));
            return std::nullopt;
        }

        mTokens->advance();

        auto right = parseBinaryExpression(precedence + 1);
        if (!right) {
            return std::nullopt;
        }

        auto expression = std::make_unique<JunoPL::Expression>();
        expression->value = JunoPL::BinaryOp{operation.value(),
                                             std::move(left.value()),
                                             std::move(right.value())};
        left = std::move(expression);
    }

    return left;
}

std::optional<ExpressionHandle> Parser::parseTernaryExpression() {
    auto condition = parseBinaryExpression(0);
    if (!condition) {
        return std::nullopt;
    }

    if (mTokens->empty() || mTokens->current().type != TokenType::OP_QUESTION) {
        return condition;
    }

    mTokens->advance();

    auto valueIfTrue = parseTernaryExpression();
    if (!valueIfTrue) {
        return std::nullopt;
    }

    if (!expectToken(TokenType::OP_COLON)) {
        return std::nullopt;
    }

    auto valueIfFalse = parseTernaryExpression();
    if (!valueIfFalse) {
        return std::nullopt;
    }

    auto expression = std::make_unique<JunoPL::Expression>();
    expression->value = JunoPL::TernaryOp{std::move(condition.value()),
                                          std::move(valueIfTrue.value()),
                                          std::move(valueIfFalse.value())};
    return expression;
}

std::optional<ExpressionHandle> Parser::parseExpression(TokenList &tokens) {
    mTokens = &tokens;
    return parseTernaryExpression();
}
}

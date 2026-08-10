module;
#include <expected>
#include <memory>
#include <utility>
module junopl.parser.handlers;

namespace JunoPL {

namespace {
using ExpressionHandle = JunoPL::ExpressionHandle;

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

std::expected<BinaryOp::Operation, ParserError>
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
        break;
    }

    return std::unexpected(ParserError::UnexpectedToken(
        token, {TokenType::OP_PLUS,        TokenType::OP_MINUS,
                TokenType::OP_MULT,        TokenType::OP_DIVIDE,
                TokenType::OP_MOD,         TokenType::K_AND,
                TokenType::K_OR,           TokenType::OP_COMP_EQ,
                TokenType::OP_COMP_NOT_EQ, TokenType::OP_COMP_GREATER,
                TokenType::OP_COMP_LESS,   TokenType::OP_COMP_GREATER_EQ,
                TokenType::OP_COMP_LESS_EQ}));
}

std::expected<UnaryOp::Operation, ParserError>
unaryOperationForToken(const Token &token) {
    switch (token.type) {
    case TokenType::K_NOT:
        return UnaryOp::Operation::NOT;
    case TokenType::OP_MINUS:
        return UnaryOp::Operation::NEGATE;
    default:
        break;
    }

    return std::unexpected(ParserError::UnexpectedToken(
        token, {TokenType::K_NOT, TokenType::OP_MINUS}));
}

std::expected<ExpressionHandle, ParserError> parseExpression(TokenList &tokens);

std::expected<ExpressionHandle, ParserError>
parsePrimaryExpression(TokenList &tokens) {
    if (tokens.empty()) {
        return std::unexpected(ParserError::EmptyTokenList(TokenType::IDENT));
    }

    switch (tokens.current().type) {
    case TokenType::NUM:
    case TokenType::STR:
    case TokenType::TRUE:
    case TokenType::FALSE:
    case TokenType::IDENT: {
        auto valueToken = tokens.current();
        tokens.advance();

        auto expression = std::make_unique<JunoPL::Expression>();
        expression->value = JunoPL::Value{valueToken.value};
        return expression;
    }
    case TokenType::OP_PAR_OP: {
        tokens.advance();
        auto expression = parseExpression(tokens);
        if (!expression) {
            return std::unexpected(expression.error());
        }

        auto closeParen = expectToken(tokens, TokenType::OP_PAR_CLO);
        if (!closeParen) {
            return std::unexpected(closeParen.error());
        }

        return expression;
    }
    default:
        return std::unexpected(ParserError::UnexpectedToken(
            tokens.current(), {TokenType::NUM, TokenType::STR, TokenType::TRUE,
                               TokenType::FALSE, TokenType::IDENT,
                               TokenType::OP_PAR_OP}));
    }
}

std::expected<ExpressionHandle, ParserError>
parseUnaryExpression(TokenList &tokens) {
    if (tokens.empty()) {
        return std::unexpected(ParserError::EmptyTokenList(TokenType::IDENT));
    }

    switch (tokens.current().type) {
    case TokenType::K_NOT:
    case TokenType::OP_MINUS: {
        auto operation = unaryOperationForToken(tokens.current());
        if (!operation) {
            return std::unexpected(operation.error());
        }

        tokens.advance();
        auto value = parseUnaryExpression(tokens);
        if (!value) {
            return std::unexpected(value.error());
        }

        auto expression = std::make_unique<JunoPL::Expression>();
        expression->value =
            JunoPL::UnaryOp{operation.value(), std::move(value.value())};
        return expression;
    }
    default:
        return parsePrimaryExpression(tokens);
    }
}

std::expected<ExpressionHandle, ParserError>
parseBinaryExpression(TokenList &tokens, int minimumPrecedence) {
    auto left = parseUnaryExpression(tokens);
    if (!left) {
        return std::unexpected(left.error());
    }

    while (!tokens.empty()) {
        int precedence = binaryPrecedence(tokens.current().type);
        if (precedence < minimumPrecedence) {
            break;
        }

        auto operation = binaryOperationForToken(tokens.current());
        if (!operation) {
            return std::unexpected(operation.error());
        }

        tokens.advance();

        auto right = parseBinaryExpression(tokens, precedence + 1);
        if (!right) {
            return std::unexpected(right.error());
        }

        auto expression = std::make_unique<JunoPL::Expression>();
        expression->value = JunoPL::BinaryOp{operation.value(),
                                             std::move(left.value()),
                                             std::move(right.value())};
        left = std::move(expression);
    }

    return left;
}

std::expected<ExpressionHandle, ParserError>
parseTernaryExpression(TokenList &tokens) {
    auto condition = parseBinaryExpression(tokens, 0);
    if (!condition) {
        return std::unexpected(condition.error());
    }

    if (tokens.empty() || tokens.current().type != TokenType::OP_QUESTION) {
        return condition;
    }

    tokens.advance();

    auto valueIfTrue = parseExpression(tokens);
    if (!valueIfTrue) {
        return std::unexpected(valueIfTrue.error());
    }

    auto colon = expectToken(tokens, TokenType::OP_COLON);
    if (!colon) {
        return std::unexpected(colon.error());
    }

    auto valueIfFalse = parseExpression(tokens);
    if (!valueIfFalse) {
        return std::unexpected(valueIfFalse.error());
    }

    auto expression = std::make_unique<JunoPL::Expression>();
    expression->value = JunoPL::TernaryOp{std::move(condition.value()),
                                          std::move(valueIfTrue.value()),
                                          std::move(valueIfFalse.value())};
    return expression;
}

std::expected<ExpressionHandle, ParserError>
parseExpression(TokenList &tokens) {
    return parseTernaryExpression(tokens);
}
} // namespace

std::expected<JunoPL::ExpressionHandle, JunoPL::ParserError>
parseSimpleValueExpression(JunoPL::TokenList &tokens) {
    return parseExpression(tokens);
}
}
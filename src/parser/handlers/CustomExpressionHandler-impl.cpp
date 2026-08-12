module;
#include <optional>
#include <utility>
module junopl.parser;
import junopl.lexer.tokens;
import junopl.parser.error;
import junopl.parser.nodes;
import junopl.parser.nodes.expressions;

namespace JunoPL {
std::optional<CustomExpression>
Parser::parseCustomExpression(TokenList &tokens) {
    mTokens = &tokens;

    if (!expectToken(TokenType::K_EXPR)) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    auto ident = expectToken(TokenType::IDENT);
    if (!ident) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    auto params = parseParam(tokens);
    if (!params) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    if (!expectToken(TokenType::OP_EQUAL)) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    auto expression = parseExpression(tokens);
    if (!expression) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    if (!expectToken(TokenType::OP_SEMICOLON)) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    CustomExpression exprNode;
    exprNode.identifier = ident->value;
    exprNode.params = std::move(params.value());
    exprNode.expression = std::move(expression.value());
    return exprNode;
}
}

module;
#include <expected>
#include <utility>
module junopl.parser.handlers;
import junopl.lexer.tokens;
import junopl.parser.error;
import junopl.parser.nodes;
import junopl.parser.nodes.expressions;

namespace JunoPL {
std::expected<CustomExpression, ParserError>
parseCustomExpression(TokenList &tokens) {
    if (auto expr = expectToken(tokens, TokenType::K_EXPR); !expr) {
        return std::unexpected(expr.error());
    }

    auto ident = expectToken(tokens, TokenType::IDENT);
    if (!ident) {
        return std::unexpected(ident.error());
    }

    auto params = parseParam(tokens);
    if (!params) {
        return std::unexpected(params.error());
    }

    auto equals = expectToken(tokens, TokenType::OP_EQUAL);
    if (!equals) {
        return std::unexpected(equals.error());
    }

    auto expression = parseExpression(tokens);
    if (!expression) {
        return std::unexpected(expression.error());
    }

    auto semicolon = expectToken(tokens, TokenType::OP_SEMICOLON);
    if (!semicolon) {
        return std::unexpected(semicolon.error());
    }

    CustomExpression exprNode;
    exprNode.identifier = ident.value().value;
    exprNode.params = std::move(params.value());
    exprNode.expression = std::move(expression.value());
    return exprNode;
}
}
module;
#include <optional>
module junopl.parser;
import junopl.lexer.tokens;
import junopl.parser.error;
import junopl.parser.nodes;
import junopl.parser.nodes.statements;
import junopl.parser.nodes.expressions;

namespace JunoPL {
std::optional<VarDeclaration> Parser::parseVarDeclaration(TokenList &tokens) {
    mTokens = &tokens;

    if (!expectToken(TokenType::K_VAR)) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    auto ident = expectToken(TokenType::IDENT);
    if (!ident) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    VarDeclaration node;
    node.identifier = ident->value;
    auto next =
        expectToken({TokenType::OP_EQUAL, TokenType::OP_SEMICOLON});
    if (!next) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    if (next->type == TokenType::OP_SEMICOLON) {
        return node;
    }

    auto value = parseExpression(tokens);
    if (!value) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    if (!expectToken(TokenType::OP_SEMICOLON)) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    node.value = std::move(value.value());
    return node;
}
}

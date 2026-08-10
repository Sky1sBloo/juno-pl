module;
#include <expected>
module junopl.parser.handlers;

import junopl.lexer.tokens;
import junopl.parser.error;
import junopl.parser.nodes;
import junopl.parser.nodes.statements;
import junopl.parser.nodes.expressions;

namespace JunoPL {
std::expected<VarDeclaration, ParserError>
parseVarDeclaration(TokenList &tokens) {
    if (auto kVar = expectToken(tokens, TokenType::K_VAR); !kVar) {
        return std::unexpected(kVar.error());
    }

    auto ident = expectToken(tokens, TokenType::IDENT);
    if (!ident) {
        return std::unexpected(ident.error());
    }

    VarDeclaration node;
    node.identifier = ident.value().value;
    auto next =
        expectToken(tokens, {TokenType::OP_EQUAL, TokenType::OP_SEMICOLON});
    if (!next) {
        return std::unexpected(next.error());
    }

    if (next.value().type == TokenType::OP_SEMICOLON) {
        return node;
    }

    auto value = parseExpression(tokens);
    if (!value) {
        return std::unexpected(value.error());
    }

    auto semicolon = expectToken(tokens, TokenType::OP_SEMICOLON);
    if (!semicolon) {
        return std::unexpected(semicolon.error());
    }

    node.value = std::move(value.value());
    return node;
}
}

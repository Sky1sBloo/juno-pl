module;
#include <expected>
#include <memory>
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
        auto expr = std::make_unique<Expression>();
        expr->value = Value{next.value().value};
        node.value = std::move(expr);
        return node;
    }

    // add handling for expression
}
}

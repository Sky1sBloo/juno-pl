module;
#include <optional>
module junopl.parser;

import junopl.lexer.tokens;
import junopl.parser.error;
import junopl.parser.nodes;
import junopl.parser.nodes.expressions;
import junopl.parser.nodes.statements;

namespace JunoPL {
std::optional<ListDeclaration> Parser::parseListDeclaration(TokenList &tokens) {
    mTokens = &tokens;

    if (!expectToken(TokenType::K_LIST)) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    auto ident = expectToken(TokenType::IDENT);
    if (!ident) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    ListDeclaration node;
    node.identifier = ident->value;

    auto next = expectToken({TokenType::OP_EQUAL, TokenType::OP_SEMICOLON});
    if (!next) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    if (next->type == TokenType::OP_SEMICOLON) {
        return node;
    }

    if (!expectToken(TokenType::OP_BRAC_OP)) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    if (mTokens->current().type != TokenType::OP_BRAC_CLO) {
        while (true) {
            auto value = parseExpression(tokens);
            if (!value) {
                recoverTo(TokenType::OP_SEMICOLON);
                return std::nullopt;
            }

            node.values.push_back(std::move(value.value()));

            if (mTokens->current().type == TokenType::OP_COMMA) {
                mTokens->advance();
                continue;
            }

            if (mTokens->current().type == TokenType::OP_BRAC_CLO) {
                break;
            }

            mErrors.emplace_back(ParserError::UnexpectedToken(
                mTokens->current(),
                {TokenType::OP_COMMA, TokenType::OP_BRAC_CLO}));
            recoverTo(TokenType::OP_SEMICOLON);
            return std::nullopt;
        }
    }

    if (!expectToken(TokenType::OP_BRAC_CLO)) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    if (!expectToken(TokenType::OP_SEMICOLON)) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    return node;
}
}

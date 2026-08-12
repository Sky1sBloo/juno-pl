module;
#include <optional>
module junopl.parser;
import junopl.lexer.tokens;
import junopl.parser.error;
import junopl.parser.nodes.expressions;
import junopl.parser.nodes.statements;

namespace JunoPL {
std::optional<EventNode> Parser::parseEvent(TokenList &tokens) {
    mTokens = &tokens;

    if (!expectToken(TokenType::K_ON)) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    auto identifier = expectToken(TokenType::IDENT);
    if (!identifier) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    auto params = parseParam(tokens);
    if (!params) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    auto body = parseBody(tokens);
    if (!body) {
        return std::nullopt;
    }

    EventNode node;
    node.identifier = identifier->value;
    node.params = std::move(params.value());
    node.body = std::move(body.value());
    return node;
}
}

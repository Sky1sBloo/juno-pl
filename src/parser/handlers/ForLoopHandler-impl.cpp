module;
#include <optional>
module junopl.parser;

namespace JunoPL {
std::optional<ForLoop> Parser::parseForLoop(TokenList &tokens) {
    mTokens = &tokens;

    if (!expectToken(TokenType::K_FOR)) {
        recoverTo(TokenType::OP_CBRAC_CLO);
        return std::nullopt;
    }

    auto iterator = expectToken(TokenType::IDENT);
    if (!iterator) {
        recoverTo(TokenType::OP_CBRAC_CLO);
        return std::nullopt;
    }

    if (!expectToken(TokenType::K_FROM)) {
        recoverTo(TokenType::OP_CBRAC_CLO);
        return std::nullopt;
    }

    auto start = parseExpression(tokens);
    if (!start) {
        recoverTo(TokenType::OP_CBRAC_CLO);
        return std::nullopt;
    }

    if (!expectToken(TokenType::K_TO)) {
        recoverTo(TokenType::OP_CBRAC_CLO);
        return std::nullopt;
    }

    auto end = parseExpression(tokens);
    if (!end) {
        recoverTo(TokenType::OP_CBRAC_CLO);
        return std::nullopt;
    }

    if (!expectToken(TokenType::K_BY)) {
        recoverTo(TokenType::OP_CBRAC_CLO);
        return std::nullopt;
    }

    auto increment = parseExpression(tokens);
    if (!increment) {
        recoverTo(TokenType::OP_CBRAC_CLO);
        return std::nullopt;
    }

    auto body = parseBody(tokens);
    if (!body) {
        return std::nullopt;
    }

    return ForLoop{iterator->value, std::move(start.value()),
                   std::move(end.value()), std::move(increment.value())};
}
}

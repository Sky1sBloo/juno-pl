module;
#include <optional>
module junopl.parser;

namespace JunoPL {
std::optional<RepeatLoop> Parser::parseRepeatLoop(TokenList &tokens) {
    mTokens = &tokens;

    if (!expectToken(TokenType::K_REPEAT)) {
        recoverTo(TokenType::OP_CBRAC_CLO);
        return std::nullopt;
    }

    auto expr = parseExpression(tokens);
    if (!expr) {
        recoverTo(TokenType::OP_CBRAC_CLO);
        return std::nullopt;
    }

    auto body = parseBody(tokens);
    if (!body) {
        return std::nullopt;
    }

    return RepeatLoop{std::move(expr.value()), std::move(body.value())};
}
}

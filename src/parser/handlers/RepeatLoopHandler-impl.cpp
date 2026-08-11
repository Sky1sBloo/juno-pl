module;
#include <expected>
module junopl.parser.handlers;

namespace JunoPL {
std::expected<RepeatLoop, ParserError> parseRepeatLoop(TokenList &tokens) {
    if (auto repeat = expectToken(tokens, TokenType::K_REPEAT); !repeat) {
        return std::unexpected(repeat.error());
    }

    auto expr = parseExpression(tokens);
    if (!expr) {
        return std::unexpected(expr.error());
    }

    auto body = parseBody(tokens);
    if (!body) {
        return std::unexpected(body.error());
    }

    return RepeatLoop{std::move(expr.value()), std::move(body.value())};
}
}
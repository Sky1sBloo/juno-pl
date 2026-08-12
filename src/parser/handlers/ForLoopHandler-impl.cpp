module;
#include <expected>
module junopl.parser.handlers;

namespace JunoPL {
std::expected<ForLoop, ParserError> parseForLoop(TokenList &tokens) {
    if (auto repeat = expectToken(tokens, TokenType::K_FOR); !repeat) {
        return std::unexpected(repeat.error());
    }

    auto iterator = expectToken(tokens, TokenType::IDENT);
    if (!iterator) {
        return std::unexpected(iterator.error());
    }

    if (auto from = expectToken(tokens, TokenType::K_FROM); !from) {
        return std::unexpected(from.error());
    }

    auto start = parseExpression(tokens);
    if (!start) {
        return std::unexpected(start.error());
    }

    if (auto to = expectToken(tokens, TokenType::K_TO); !to) {
        return std::unexpected(to.error());
    }
    auto end = parseExpression(tokens);
    if (!end) {
        return std::unexpected(end.error());
    }

    if (auto by = expectToken(tokens, TokenType::K_BY); !by) {
        return std::unexpected(by.error());
    }
    auto increment = parseExpression(tokens);
    if (!increment) {
        return std::unexpected(increment.error());
    }

    auto body = parseBody(tokens);
    if (!body) {
        return std::unexpected(body.error());
    }

    return ForLoop{iterator.value().value, std::move(start.value()),
                   std::move(end.value()), std::move(increment.value())};
}
}
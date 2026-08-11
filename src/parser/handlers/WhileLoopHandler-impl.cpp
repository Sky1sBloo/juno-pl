module;
#include <expected>
module junopl.parser.handlers;

namespace JunoPL {
std::expected<WhileLoop, ParserError> parseWhileLoop(TokenList &tokens) {
    if (auto repeat = expectToken(tokens, TokenType::K_WHILE); !repeat) {
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

    return WhileLoop{std::move(expr.value()), std::move(body.value())};
}
}
#include <expected>
module junopl.parser.handlers;

namespace JunoPL {
std::expected<EmitEvent, ParserError> parseEmitEvent(TokenList &tokens) {
    if (auto emitToken = expectToken(tokens, TokenType::K_EMIT); !emitToken) {
        return std::unexpected(emitToken.error());
    }

    auto ident = expectToken(tokens, TokenType::IDENT);
    if (!ident) {
        return std::unexpected(ident.error());
    }

    auto params = parseParamExpr(tokens);
    if (!params) {
        return std::unexpected(params.error());
    }

    if (auto end = expectToken(
            tokens, {TokenType::OP_SEMICOLON, TokenType::K_TO}, false);
        !end) {
        return std::unexpected(end.error());
    }
    if (tokens.current().type == TokenType::OP_SEMICOLON) {
        tokens.advance();
        return EmitEvent{ident.value().value, std::move(params.value()),
                         EmitEvent::Options::TO_SCRIPT};
    }
    tokens.advance();

    EmitEvent::Options options;
    switch (tokens.current().type) {
    case TokenType::K_CRAFT:
        options = EmitEvent::Options::TO_CRAFT;
        break;
    case TokenType::K_NEARBY:
        options = EmitEvent::Options::TO_NEARBY_CRAFTS;
        break;
    default:
        return std::unexpected(ParserError::UnexpectedToken(
            tokens.current(), {TokenType::K_CRAFT, TokenType::K_NEARBY}));
    }
    tokens.advance();

    return EmitEvent{ident.value().value, std::move(params.value()), options};
}
}
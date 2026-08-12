module;
#include <optional>
module junopl.parser;

namespace JunoPL {
std::optional<EmitEvent> Parser::parseEmitEvent(TokenList &tokens) {
    mTokens = &tokens;

    if (!expectToken(TokenType::K_EMIT)) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    auto ident = expectToken(TokenType::IDENT);
    if (!ident) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    auto params = parseParamExpr(tokens);
    if (!params) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    auto end = expectToken({TokenType::OP_SEMICOLON, TokenType::K_TO}, false);
    if (!end) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }
    if (mTokens->current().type == TokenType::OP_SEMICOLON) {
        mTokens->advance();
        return EmitEvent{ident->value, std::move(params.value()),
                         EmitEvent::Options::TO_SCRIPT};
    }
    mTokens->advance();

    EmitEvent::Options options;
    switch (mTokens->current().type) {
    case TokenType::K_CRAFT:
        options = EmitEvent::Options::TO_CRAFT;
        break;
    case TokenType::K_NEARBY:
        options = EmitEvent::Options::TO_NEARBY_CRAFTS;
        break;
    default:
        mErrors.emplace_back(ParserError::UnexpectedToken(
            mTokens->current(), {TokenType::K_CRAFT, TokenType::K_NEARBY}));
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }
    mTokens->advance();
    if (!expectToken(TokenType::OP_SEMICOLON)) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    return EmitEvent{ident->value, std::move(params.value()), options};
}
}

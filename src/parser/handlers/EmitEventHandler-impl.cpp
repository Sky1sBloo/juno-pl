module;
#include <optional>
#include <string>
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

    std::string identifier = ident->value;

    // Check for qualified identifier: identifier.identifier
    if (!mTokens->empty() && mTokens->current().type == TokenType::OP_DOT) {
        mTokens->advance();
        auto methodName = expectToken(TokenType::IDENT);
        if (!methodName) {
            recoverTo(TokenType::OP_SEMICOLON);
            return std::nullopt;
        }
        // For emit, the qualifier is used as the identifier with dot
        identifier = ident->value + "." + methodName->value;
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
        return EmitEvent{identifier, std::move(params.value()),
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

    return EmitEvent{identifier, std::move(params.value()), options};
}
}

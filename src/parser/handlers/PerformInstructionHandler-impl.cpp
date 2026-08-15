module;
#include <optional>
module junopl.parser;

namespace JunoPL {
std::optional<PerformInstruction>
Parser::parsePerformInstruction(TokenList &tokens) {
    mTokens = &tokens;

    if (!expectToken(TokenType::K_PERFORM)) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    auto ident = expectToken(TokenType::IDENT);
    if (!ident) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    PerformInstruction instr;
    instr.identifier = ident->value;

    // Check for qualified identifier: identifier.identifier
    if (!mTokens->empty() && mTokens->current().type == TokenType::OP_DOT) {
        mTokens->advance();
        auto methodName = expectToken(TokenType::IDENT);
        if (!methodName) {
            recoverTo(TokenType::OP_SEMICOLON);
            return std::nullopt;
        }
        instr.qualifier = instr.identifier;
        instr.identifier = methodName->value;
    }

    auto params = parseParamExpr(tokens);
    if (!params) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    if (!expectToken(TokenType::OP_SEMICOLON)) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    instr.params = std::move(params.value());
    return instr;
}
}

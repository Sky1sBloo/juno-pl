module;
#include <optional>
module junopl.parser;

namespace JunoPL {
std::optional<ConditionalStatement>
Parser::parseConditionalStatement(TokenList &tokens) {
    mTokens = &tokens;

    if (!expectToken(TokenType::K_IF)) {
        recoverTo(TokenType::OP_CBRAC_CLO);
        return std::nullopt;
    }

    if (!expectToken(TokenType::OP_PAR_OP)) {
        recoverTo(TokenType::OP_CBRAC_CLO);
        return std::nullopt;
    }

    auto condition = parseExpression(tokens);
    if (!condition) {
        recoverTo(TokenType::OP_CBRAC_CLO);
        return std::nullopt;
    }

    if (!expectToken(TokenType::OP_PAR_CLO)) {
        recoverTo(TokenType::OP_CBRAC_CLO);
        return std::nullopt;
    }

    auto body = parseBody(tokens);
    if (!body) {
        return std::nullopt;
    }

    ConditionalStatement stmt;
    stmt.ifStatement =
        IfStatement{std::move(condition.value()), std::move(body.value())};

    while (!mTokens->empty() &&
           mTokens->current().type == TokenType::K_ELIF) {
        mTokens->advance();
        if (!expectToken(TokenType::OP_PAR_OP)) {
            recoverTo(TokenType::OP_CBRAC_CLO);
            return std::nullopt;
        }

        auto elifCond = parseExpression(tokens);
        if (!elifCond) {
            recoverTo(TokenType::OP_CBRAC_CLO);
            return std::nullopt;
        }

        if (!expectToken(TokenType::OP_PAR_CLO)) {
            recoverTo(TokenType::OP_CBRAC_CLO);
            return std::nullopt;
        }

        auto elifBody = parseBody(tokens);
        if (!elifBody) {
            return std::nullopt;
        }

        stmt.elifStatements.emplace_back(IfStatement{
            std::move(elifCond.value()), std::move(elifBody.value())});
    }

    if (mTokens->empty() || mTokens->current().type != TokenType::K_ELSE) {
        return stmt;
    }
    mTokens->advance();

    auto elseBody = parseBody(tokens);
    if (!elseBody) {
        return std::nullopt;
    }
    stmt.elseStatement = std::move(elseBody.value());
    return stmt;
}
}

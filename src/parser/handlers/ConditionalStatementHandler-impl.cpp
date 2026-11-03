module;
#include <optional>
module junopl.parser;

namespace JunoPL {
std::optional<ConditionalStatement>
Parser::parseConditionalStatement(TokenList &tokens) {
    mTokens = &tokens;

    auto ifToken = expectToken(TokenType::K_IF);
    if (!ifToken) {
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
    stmt.ifStatement = IfStatement{std::move(condition.value()),
                                   std::move(body.value()),
                                   locationOf(*ifToken)};
    stmt.loc = locationOf(*ifToken);

    while (!mTokens->empty() &&
           mTokens->current().type == TokenType::K_ELIF) {
        auto elifToken = mTokens->current();
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
            std::move(elifCond.value()), std::move(elifBody.value()),
            locationOf(elifToken)});
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

module;
#include <expected>
module junopl.parser.handlers;

namespace JunoPL {
std::expected<ConditionalStatement, ParserError>
parseConditionalStatement(TokenList &tokens) {
    if (auto ifToken = expectToken(tokens, TokenType::K_IF); !ifToken) {
        return std::unexpected(ifToken.error());
    }

    if (auto openPar = expectToken(tokens, TokenType::OP_PAR_OP); !openPar) {
        return std::unexpected(openPar.error());
    }

    auto condition = parseExpression(tokens);
    if (!condition) {
        return std::unexpected(condition.error());
    }

    auto body = parseBody(tokens);
    if (!body) {
        return std::unexpected(body.error());
    }

    ConditionalStatement stmt;
    if (tokens.empty()) {
        stmt.ifStatement =
            IfStatement{std::move(condition.value()), std::move(body.value())};
        return stmt;
    }

    while (tokens.current().type == TokenType::K_ELIF) {
        tokens.advance();
        if (auto openPar = expectToken(tokens, TokenType::OP_PAR_OP);
            !openPar) {
            return std::unexpected(openPar.error());
        }

        if (auto elifCond = parseExpression(tokens); !elifCond) {
            return std::unexpected(elifCond.error());
        } else if (auto elifBody = parseBody(tokens); !elifBody) {
            return std::unexpected(elifBody.error());
        } else {
            stmt.elifStatements.emplace_back(IfStatement{
                std::move(elifCond.value()), std::move(elifBody.value())});
        }
    }

    auto elseToken = expectToken(tokens, TokenType::K_ELSE);
    if (!elseToken) {
        return stmt;
    }

    auto elseBody = parseBody(tokens);
    if (!elseBody) {
        return std::unexpected(elseBody.error());
    }
    stmt.elseStatement = std::move(elseBody.value());
    return stmt;
}
}
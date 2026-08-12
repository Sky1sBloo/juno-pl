module;
#include <optional>
#include <string>
#include <vector>
module junopl.parser;
import junopl.lexer.tokens;
import junopl.parser.error;
import junopl.parser.nodes.expressions;
import junopl.parser.nodes.statements;

namespace JunoPL {
std::optional<std::vector<std::string>> Parser::parseParam(TokenList &tokens) {
    mTokens = &tokens;

    if (!expectToken(TokenType::OP_PAR_OP)) {
        recoverTo(TokenType::OP_PAR_CLO);
        return std::nullopt;
    }

    std::vector<std::string> params;
    while (mTokens->current().type != TokenType::OP_PAR_CLO) {
        auto ident = expectToken(TokenType::IDENT);
        if (!ident) {
            recoverTo(TokenType::OP_PAR_CLO);
            return std::nullopt;
        }

        params.push_back(ident->value);
        auto currentToken = mTokens->current();
        if (currentToken.type == TokenType::OP_COMMA) {
            mTokens->advance();
            continue;
        } else if (currentToken.type == TokenType::OP_PAR_CLO) {
            break;
        } else {
            mErrors.emplace_back(ParserError::UnexpectedToken(
                mTokens->current(),
                {TokenType::OP_COMMA, TokenType::OP_PAR_CLO}));
            recoverTo(TokenType::OP_PAR_CLO);
            return std::nullopt;
        }
    }
    if (!expectToken(TokenType::OP_PAR_CLO)) {
        return std::nullopt;
    }

    return params;
}
}

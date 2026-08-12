module;
#include <optional>
#include <vector>
module junopl.parser;
import junopl.lexer.tokens;
import junopl.parser.error;
import junopl.parser.nodes.expressions;
import junopl.parser.nodes.statements;

namespace JunoPL {
std::optional<std::vector<ExpressionHandle>>
Parser::parseParamExpr(TokenList &tokens) {
    mTokens = &tokens;

    if (!expectToken(TokenType::OP_PAR_OP)) {
        recoverTo(TokenType::OP_PAR_CLO);
        return std::nullopt;
    }

    std::vector<ExpressionHandle> paramValues;
    while (mTokens->current().type != TokenType::OP_PAR_CLO) {
        auto value = parseExpression(tokens);
        if (!value) {
            recoverTo(TokenType::OP_PAR_CLO);
            return std::nullopt;
        }

        paramValues.push_back(std::move(value.value()));
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
    return paramValues;
}
}

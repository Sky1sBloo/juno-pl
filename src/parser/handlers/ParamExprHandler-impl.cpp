module;
#include <expected>
#include <vector>
module junopl.parser.handlers;
import junopl.lexer.tokens;
import junopl.parser.error;
import junopl.parser.nodes.expressions;
import junopl.parser.nodes.statements;

namespace JunoPL {
std::expected<std::vector<ExpressionHandle>, ParserError>
parseParamExpr(TokenList &tokens) {
    if (auto openPar = expectToken(tokens, TokenType::OP_PAR_OP); !openPar) {
        return std::unexpected(openPar.error());
    }

    std::vector<ExpressionHandle> paramValues;
    while (tokens.current().type != TokenType::OP_BRAC_CLO) {
        auto value = parseExpression(tokens);
        if (!value) {
            return std::unexpected(value.error());
        }

        paramValues.push_back(std::move(value.value()));
        auto currentToken = tokens.current();
        if (currentToken.type == TokenType::OP_COMMA) {
            continue;
        } else if (currentToken.type == TokenType::OP_PAR_CLO) {
            break;
        } else {
            return std::unexpected(ParserError::UnexpectedToken(
                tokens.current(),
                {TokenType::OP_COMMA, TokenType::OP_PAR_CLO}));
        }
    }

    if (auto cloPar = expectToken(tokens, TokenType::OP_PAR_CLO); !cloPar) {
        return std::unexpected(cloPar.error());
    }
    return paramValues;
}
}
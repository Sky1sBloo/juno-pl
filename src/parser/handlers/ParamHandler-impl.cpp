module;
#include <expected>
#include <string>
#include <vector>
module junopl.parser.handlers;
import junopl.lexer.tokens;
import junopl.parser.error;
import junopl.parser.nodes.expressions;
import junopl.parser.nodes.statements;

namespace JunoPL {
std::expected<std::vector<std::string>, ParserError>
parseParam(TokenList &tokens) {
    auto parOp = expectToken(tokens, TokenType::OP_PAR_OP);
    if (!parOp)
        return std::unexpected(parOp.error());

    std::vector<std::string> params;
    while (tokens.current().type != TokenType::OP_PAR_CLO) {
        auto ident = expectToken(tokens, TokenType::IDENT);
        if (!ident)
            return std::unexpected(parOp.error());

        params.push_back(ident->value);
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
    auto parClo = expectToken(tokens, TokenType::OP_PAR_CLO);
    if (!parClo)
        return std::unexpected(parOp.error());

    return params;
}
}

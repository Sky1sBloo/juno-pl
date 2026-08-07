module;
#include <expected>
module junopl.parser.handlers;
import junopl.lexer.tokens;
import junopl.parser.error;
import junopl.parser.nodes;
import junopl.parser.nodes.statements;

namespace JunoPL {
std::expected<Body, ParserError> parseBody(TokenList &tokens) {
    auto openBrace = expectToken(tokens, TokenType::OP_CBRAC_OP);
    if (!openBrace)
        return std::unexpected(openBrace.error());

    Body body;

    if (tokens.current().type == TokenType::OP_CBRAC_CLO) {
        auto closeBrace = expectToken(tokens, TokenType::OP_CBRAC_CLO);
        if (!closeBrace)
            return std::unexpected(closeBrace.error());
        return body;
    }

    return std::unexpected(ParserError::UnexpectedToken(
        tokens.current(), {TokenType::OP_CBRAC_CLO}));
}
}
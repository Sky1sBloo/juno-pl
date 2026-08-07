module;
#include <expected>
export module junopl.parser.handlers;

import junopl.lexer.tokens;
import junopl.parser.error;
import junopl.parser.nodes;

namespace JunoPL {
export std::expected<Token, ParserError>
expectToken(TokenList &tokens, TokenType expectedToken, bool advance = true) {
    if (tokens.empty()) {
        return std::unexpected(ParserError::EmptyTokenList(expectedToken));
    }

    Token token = tokens.current();
    if (token.type != expectedToken) {
        return std::unexpected(
            ParserError::UnexpectedToken(token, expectedToken));
    }

    if (advance) {
        tokens.advance();
    }
    return token;
}

export std::expected<FunctionNode, ParserError>
parseFunctionHandler(TokenList &tokens);
}
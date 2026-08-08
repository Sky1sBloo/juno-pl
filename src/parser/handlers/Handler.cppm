module;
#include <expected>
#include <initializer_list>
#include <string>
#include <vector>
export module junopl.parser.handlers;

import junopl.lexer.tokens;
import junopl.parser.error;
import junopl.parser.nodes;
import junopl.parser.nodes.statements;

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

export std::expected<Token, ParserError>
expectToken(TokenList &tokens, std::initializer_list<TokenType> expectedTokens,
            bool advance = true) {
    if (tokens.empty()) {
        return std::unexpected(ParserError::EmptyTokenList(expectedTokens));
    }

    Token token = tokens.current();
    bool found = false;
    for (TokenType expectedToken: expectedTokens) {
        if (token.type == expectedToken) {
            found = true;
        }
    }
    if (!found) {
        return std::unexpected(ParserError::UnexpectedToken(token, expectedTokens));
    }

    if (advance) {
        tokens.advance();
    }
    return token;
}

export std::expected<FunctionNode, ParserError>
parseFunction(TokenList &tokens);

export std::expected<EventNode, ParserError> parseEvent(TokenList &tokens);

export std::expected<std::vector<std::string>, ParserError>
parseParam(TokenList &tokens);

export std::expected<Body, ParserError> parseBody(TokenList &tokens);

export std::expected<VarDeclaration, ParserError>
parseVarDeclaration(TokenList &tokens);
}
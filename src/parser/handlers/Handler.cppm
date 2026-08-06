module;
#include <expected>
#include <memory>
export module junopl.parser.handlers;

import junopl.lexer.tokens;
import junopl.parser.error;
import junopl.parser.nodes.expressions;
import junopl.parser.nodes.statements;
;

namespace JunoPL {
/**
Base class for handling expressions */
export class ExpressionHandler {
  public:
    /**
    Parses a span of tokens
     */
    virtual std::expected<std::unique_ptr<Expression>, ParserError>
    parse(TokenList &tokens) = 0;

  private:
    // Save handlers here
};

export class StatementHandler {
  public:
    virtual std::expected<std::unique_ptr<Statements>, ParserError>
    parse(TokenList &tokens) = 0;

  private:
    // Save handlers here
};

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
}
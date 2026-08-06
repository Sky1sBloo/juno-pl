module;
#include <expected>
#include <memory>
#include <span>
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
    parse(const std::span<Token> tokens) = 0;

  private:
    // Save handlers here
};

export class StatementHandler {
  public:
    virtual std::expected<std::unique_ptr<Statements>, ParserError>
    parse(const std::span<Token> tokens) = 0;

  private:
    // Save handlers here
};
}
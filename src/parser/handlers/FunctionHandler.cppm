module;
#include <expected>
#include <memory>
export module junopl.parser.handlers.function;
import junopl.parser.handlers;
import junopl.lexer.tokens;
import junopl.parser.error;
import junopl.parser.nodes.expressions;
import junopl.parser.nodes.statements;

namespace JunoPL {
export std::expected<std::unique_ptr<Statements>, ParserError>
parseFunctionHandler(TokenList &tokens);
}
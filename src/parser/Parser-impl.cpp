module;
#include <expected>
#include <vector>
module junopl.parser;
import junopl.lexer.tokens;
import junopl.parser.nodes;
import junopl.parser.error;

namespace JunoPL {
std::expected<RootNode, ParserError> Parser::parse(const std::vector<Token> &tokens) {
    if (tokens.empty()) {
        return std::unexpected(ParserError{ParserError::Type::EMPTY_TOKENS, "Parser did not receive any tokens"});
    }
}
}

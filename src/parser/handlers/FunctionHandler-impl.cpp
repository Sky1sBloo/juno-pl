module;
#include <expected>
#include <memory>
module junopl.parser.handlers;
import junopl.lexer.tokens;
import junopl.parser.error;
import junopl.parser.nodes.expressions;
import junopl.parser.nodes.statements;

namespace JunoPL {
std::expected<FunctionNode, ParserError>
parseFunctionHandler(TokenList &tokens) {
    auto instr = expectToken(tokens, TokenType::K_INSTR);
    if (!instr.has_value()) {
        return std::unexpected(instr.error());
    }

    auto identifier = expectToken(tokens, TokenType::IDENT);
    if (!identifier.has_value()) {
        return std::unexpected(identifier.error());
    }

    // handle params
    // handle body
}
}
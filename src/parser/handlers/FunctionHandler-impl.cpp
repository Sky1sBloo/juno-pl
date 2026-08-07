module;
#include <expected>
module junopl.parser.handlers;
import junopl.lexer.tokens;
import junopl.parser.error;
import junopl.parser.nodes.expressions;
import junopl.parser.nodes.statements;

namespace JunoPL {
std::expected<FunctionNode, ParserError> parseFunction(TokenList &tokens) {
    if (auto instr = expectToken(tokens, TokenType::K_INSTR); !instr) {
        return std::unexpected(instr.error());
    }

    if (auto identifier = expectToken(tokens, TokenType::IDENT); !identifier) {
        return std::unexpected(identifier.error());
    } else {
        // identifier is available through identifier->value
        if (auto params = parseParam(tokens); !params) {
            return std::unexpected(params.error());
        } else if (auto body = parseBody(tokens); !body) {
            return std::unexpected(body.error());
        } else {
            FunctionNode node;
            node.identifier = identifier->value;
            node.params = std::move(params.value());
            node.body = std::move(body.value());
            return node;
        }
    }
}
}
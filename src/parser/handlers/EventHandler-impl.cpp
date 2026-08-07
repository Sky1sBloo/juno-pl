module;
#include <expected>
module junopl.parser.handlers;
import junopl.lexer.tokens;
import junopl.parser.error;
import junopl.parser.nodes.expressions;
import junopl.parser.nodes.statements;

namespace JunoPL {
std::expected<EventNode, ParserError> parseEvent(TokenList &tokens) {
    if (auto on = expectToken(tokens, TokenType::K_ON); !on) {
        return std::unexpected(on.error());
    }

    if (auto identifier = expectToken(tokens, TokenType::IDENT); !identifier) {
        return std::unexpected(identifier.error());
    } else {
        if (auto params = parseParam(tokens); !params) {
            return std::unexpected(params.error());
        } else if (auto body = parseBody(tokens); !body) {
            return std::unexpected(body.error());
        } else {
            EventNode node;
            node.identifier = identifier.value().value;
            node.params = std::move(params.value());
            node.body = std::move(body.value());
            return node;
        }
    }
}
}
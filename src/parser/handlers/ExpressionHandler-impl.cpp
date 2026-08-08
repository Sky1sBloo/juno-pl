module;
#include <expected>
module junopl.parser.handlers;
import junopl.lexer.tokens;
import junopl.parser.error;
import junopl.parser.nodes;
import junopl.parser.nodes.expressions;

namespace JunoPL {
std::expected<CustomExpression, ParserError>
parseCustomExpression(TokenList &tokens) {
    if (auto expr = expectToken(tokens, TokenType::K_EXPR); !expr) {
        return std::unexpected(expr.error());
    }

    if (auto ident = expectToken(tokens, TokenType::IDENT); !ident) {
        return std::unexpected(ident.error());
    } else {
        if (auto params = parseParam(tokens); !params) {
            return std::unexpected(params.error());
        } else {
            // todo: handle expression value content
            CustomExpression exprNode;
            exprNode.identifier = ident.value().value;
            exprNode.params = std::move(params.value());
            return exprNode;
        }
    }
}
}
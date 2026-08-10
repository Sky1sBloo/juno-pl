module;
#include <expected>
#include <vector>
module junopl.parser.handlers;
import junopl.lexer.tokens;
import junopl.parser.error;
import junopl.parser.nodes.expressions;
import junopl.parser.nodes.statements;

namespace JunoPL {
std::expected<std::vector<ExpressionHandle>, ParserError>
parseParamExpr(TokenList &tokens) {
    if (auto openPar = expectToken(tokens, TokenType::OP_PAR_OP); !openPar) {
        return std::unexpected(openPar.error());
    }

    std::vector<ExpressionHandle> paramValues;
    while (tokens.current().type != TokenType::OP_BRAC_CLO) {
    }
}
}
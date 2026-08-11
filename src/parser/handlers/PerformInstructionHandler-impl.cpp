module;
#include <expected>
module junopl.parser.handlers;

namespace JunoPL {
std::expected<PerformInstruction, ParserError>
parsePerformInstruction(TokenList &tokens) {
    if (auto perform = expectToken(tokens, TokenType::K_PERFORM); !perform) {
        return std::unexpected(perform.error());
    }

    auto ident = expectToken(tokens, TokenType::IDENT);
    if (!ident) {
        return std::unexpected(ident.error());
    }

    auto params = parseParamExpr(tokens);
    if (!params) {
        return std::unexpected(params.error());
    }

    if (auto semiColon = expectToken(tokens, TokenType::OP_SEMICOLON);
        !semiColon) {
        return std::unexpected(semiColon.error());
    }

    PerformInstruction instr;
    instr.identifier = ident.value().value;
    instr.params = std::move(params.value());
    return instr;
}
}

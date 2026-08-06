module;
#include <expected>
#include <string>
module junopl.parser;
import junopl.lexer.tokens;
import junopl.parser.nodes;
import junopl.parser.error;
import junopl.parser.handlers;

namespace JunoPL {
Parser::Output Parser::parse(TokenList &tokens) {
    Output output;
    if (tokens.empty()) {
        output.errors.emplace_back(ParserError{ParserError::Type::EMPTY_TOKENS,
                                               "Parser received empty tokens"});
        return output;
    }

    RootNode root;
    switch (tokens.current().type) {
    case TokenType::K_PROGRAM: {
        auto programName = handleProgramName(tokens);
        if (programName.has_value()) {
            root.programName = programName.value();
        } else {
            output.errors.push_back(programName.error());
        }
        break;
    }
    case TokenType::K_IMPORT: {
        auto importPath = handleImport(tokens);
        if (importPath.has_value()) {
            root.body.push_back(importPath.value());
        }
    } break;
    }
    return output;
}

std::expected<std::string, ParserError>
Parser::handleProgramName(TokenList &tokens) {
    auto program = expectToken(tokens, TokenType::K_PROGRAM);
    if (!program.has_value()) {
        return std::unexpected(program.error());
    }

    auto name = expectToken(tokens, TokenType::STR);
    if (!name.has_value()) {
        return std::unexpected(program.error());
    }
    return name.value().value;
}

std::expected<ImportNode, ParserError> Parser::handleImport(TokenList &tokens) {
    auto importSymbol = expectToken(tokens, TokenType::K_IMPORT);
    if (!importSymbol.has_value()) {
        return std::unexpected(importSymbol.error());
    }

    auto importPath = expectToken(tokens, TokenType::STR);
    if (!importPath.has_value()) {
        return std::unexpected(importPath.error());
    }
    ImportNode node;
    node.path = importPath.value().value;
    return node;
}
}
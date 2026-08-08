module;
#include <expected>
#include <memory>
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
        } else {
            output.errors.push_back(importPath.error());
        }
    } break;
    case TokenType::K_INSTR: {
        if (auto instr = parseFunction(tokens); !instr) {
            output.errors.push_back(instr.error());
        } else {
            root.body.push_back(std::move(instr.value()));
        }
        break;
    }
    case TokenType::K_ON: {
        if (auto on = parseFunction(tokens); !on) {
            output.errors.push_back(on.error());
        } else {
            root.body.push_back(std::move(on.value()));
        }
        break;
    }
    case TokenType::K_VAR: {
        if (auto var = parseVarDeclaration(tokens); !var) {
            output.errors.push_back(var.error());
        } else {
            root.body.push_back(std::move(var.value()));
        }
        break;
    }
    case TokenType::K_LIST: {
        if (auto list = parseListDeclaration(tokens); !list) {
            output.errors.push_back(list.error());
        } else {
            root.body.push_back(std::move(list.value()));
        }
        break;
    }
    case TokenType::K_EXPR: {
        if (auto expr = parseCustomExpression(tokens); !expr) {
            output.errors.push_back(expr.error());
        } else {
            root.body.push_back(std::move(expr.value()));
        }
        break;
    }
    default: {
        output.errors.push_back(ParserError::UnexpectedToken(
            tokens.current(), {TokenType::K_PROGRAM, TokenType::K_IMPORT,
                               TokenType::K_INSTR, TokenType::K_ON,
                               TokenType::K_VAR, TokenType::K_LIST,
                               TokenType::K_EXPR}));
    }
    }

    output.root = std::make_unique<RootNode>(std::move(root));
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
        return std::unexpected(name.error());
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
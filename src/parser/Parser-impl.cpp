module;
#include <memory>
#include <optional>
#include <string>
module junopl.parser;
import junopl.lexer.tokens;
import junopl.parser.nodes;
import junopl.parser.error;

namespace JunoPL {

std::optional<Token> Parser::expectToken(TokenType expectedType, bool advance) {
    if (mTokens->empty()) {
        mErrors.emplace_back(ParserError::EmptyTokenList(expectedType));
        return std::nullopt;
    }

    Token token = mTokens->current();
    if (token.type != expectedType) {
        mErrors.emplace_back(ParserError::UnexpectedToken(token, expectedType));
        return std::nullopt;
    }

    if (advance) {
        mTokens->advance();
    }
    return token;
}

std::optional<Token>
Parser::expectToken(std::initializer_list<TokenType> expectedTypes,
                    bool advance) {
    if (mTokens->empty()) {
        mErrors.emplace_back(ParserError::EmptyTokenList(expectedTypes));
        return std::nullopt;
    }

    Token token = mTokens->current();
    bool found = false;
    for (TokenType type : expectedTypes) {
        if (token.type == type) {
            found = true;
            break;
        }
    }
    if (!found) {
        mErrors.emplace_back(ParserError::UnexpectedToken(token, expectedTypes));
        return std::nullopt;
    }

    if (advance) {
        mTokens->advance();
    }
    return token;
}

void Parser::recoverTo(TokenType stopType) {
    while (!mTokens->empty() && mTokens->current().type != stopType) {
        mTokens->advance();
    }
}

void Parser::pushError(ParserError error) {
    mErrors.emplace_back(std::move(error));
}

Parser::Output Parser::parse(TokenList &tokens) {
    mTokens = &tokens;
    mErrors.clear();

    Output output;
    if (tokens.empty()) {
        mErrors.emplace_back(ParserError{ParserError::Type::EMPTY_TOKENS,
                                         "Parser received empty tokens"});
        output.errors = std::move(mErrors);
        return output;
    }

    RootNode root;
    switch (tokens.current().type) {
    case TokenType::K_PROGRAM: {
        if (auto programName = handleProgramName()) {
            root.programName = programName.value();
        }
        break;
    }
    case TokenType::K_IMPORT: {
        if (auto importNode = handleImport()) {
            root.body.push_back(importNode.value());
        }
        break;
    }
    case TokenType::K_INSTR: {
        if (auto instr = parseFunction(tokens)) {
            root.body.push_back(std::move(instr.value()));
        }
        break;
    }
    case TokenType::K_ON: {
        if (auto on = parseEvent(tokens)) {
            root.body.push_back(std::move(on.value()));
        }
        break;
    }
    case TokenType::K_VAR: {
        if (auto var = parseVarDeclaration(tokens)) {
            root.body.push_back(std::move(var.value()));
        }
        break;
    }
    case TokenType::K_LIST: {
        if (auto list = parseListDeclaration(tokens)) {
            root.body.push_back(std::move(list.value()));
        }
        break;
    }
    case TokenType::K_EXPR: {
        if (auto expr = parseCustomExpression(tokens)) {
            root.body.push_back(std::move(expr.value()));
        }
        break;
    }
    default: {
        mErrors.emplace_back(ParserError::UnexpectedToken(
            tokens.current(),
            {TokenType::K_PROGRAM, TokenType::K_IMPORT, TokenType::K_INSTR,
             TokenType::K_ON, TokenType::K_VAR, TokenType::K_LIST,
             TokenType::K_EXPR}));
    }
    }

    output.root = std::make_unique<RootNode>(std::move(root));
    output.errors = std::move(mErrors);
    return output;
}

std::optional<std::string> Parser::handleProgramName() {
    if (!expectToken(TokenType::K_PROGRAM)) {
        return std::nullopt;
    }

    auto name = expectToken(TokenType::STR);
    if (!name) {
        return std::nullopt;
    }
    return name->value;
}

std::optional<ImportNode> Parser::handleImport() {
    if (!expectToken(TokenType::K_IMPORT)) {
        return std::nullopt;
    }

    auto importPath = expectToken(TokenType::STR);
    if (!importPath) {
        return std::nullopt;
    }
    ImportNode node;
    node.path = importPath->value;
    return node;
}
}

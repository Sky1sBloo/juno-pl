module;
#include <memory>
#include <optional>
#include <string>
#include <vector>
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

SourceLocation Parser::locationOf(const Token &token) {
    return SourceLocation{token.line, token.colStart, token.colEnd};
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

    // Parse program name
    if (tokens.current().type == TokenType::K_PROGRAM) {
        root.loc = locationOf(tokens.current());
        if (auto programName = handleProgramName()) {
            root.programName = programName.value();
        }
    } else {
        mErrors.emplace_back(ParserError::UnexpectedToken(
            tokens.current(), TokenType::K_PROGRAM));
        output.root = std::make_unique<RootNode>(std::move(root));
        output.errors = std::move(mErrors);
        return output;
    }

    // Parse imports
    while (!mTokens->empty() && mTokens->current().type == TokenType::K_IMPORT) {
        if (auto importNode = handleImport()) {
            root.body.push_back(importNode.value());
        }
    }

    // Parse root tokens
    while (!mTokens->empty()) {
        switch (mTokens->current().type) {
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
                mTokens->current(),
                {TokenType::K_INSTR, TokenType::K_ON, TokenType::K_VAR,
                 TokenType::K_LIST, TokenType::K_EXPR}));
            recoverTo(TokenType::OP_SEMICOLON);
            if (!mTokens->empty()) mTokens->advance();
            break;
        }
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

    auto name = expectToken(TokenType::IDENT);
    if (!name) {
        return std::nullopt;
    }

    if (!expectToken(TokenType::OP_SEMICOLON)) {
        return std::nullopt;
    }

    return name->value;
}

std::optional<ImportNode> Parser::handleImport() {
    auto importToken = expectToken(TokenType::K_IMPORT);
    if (!importToken) {
        return std::nullopt;
    }

    auto importPath = expectToken(TokenType::STR);
    if (!importPath) {
        return std::nullopt;
    }

    if (!expectToken(TokenType::OP_SEMICOLON)) {
        return std::nullopt;
    }

    ImportNode node;
    node.path = importPath->value;
    node.loc = locationOf(*importToken);
    return node;
}

std::optional<VarAssignment> Parser::parseVarAssignment(TokenList &tokens) {
    mTokens = &tokens;

    auto ident = expectToken(TokenType::IDENT);
    if (!ident) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    VarAssignment node;
    node.identifier = ident->value;
    node.loc = locationOf(*ident);

    // Check for list index: identifier[expr]
    if (!mTokens->empty() && mTokens->current().type == TokenType::OP_BRAC_OP) {
        mTokens->advance();
        auto indexExpr = parseExpression(tokens);
        if (!indexExpr) {
            recoverTo(TokenType::OP_SEMICOLON);
            return std::nullopt;
        }
        if (!expectToken(TokenType::OP_BRAC_CLO)) {
            recoverTo(TokenType::OP_SEMICOLON);
            return std::nullopt;
        }
        node.indexExpr = std::move(indexExpr.value());
    }

    // An identifier followed by "." or "(" in statement position is a
    // call, and a call needs the do keyword: report that instead of
    // complaining about the assignment operator
    if (!mTokens->empty() &&
        (mTokens->current().type == TokenType::OP_DOT ||
         mTokens->current().type == TokenType::OP_PAR_OP)) {
        std::string call = ident->value;
        // Echo the statement back, stopping at its end. The lexer drops
        // the quotes around strings, so they are put back here.
        const int line = mTokens->current().line;
        for (const Token &token : mTokens->remaining()) {
            if (token.line != line ||
                token.type == TokenType::OP_SEMICOLON ||
                token.type == TokenType::OP_CBRAC_CLO) {
                break;
            }
            const bool quoted = token.type == TokenType::STR;
            if (quoted) {
                call += '"';
            }
            call += token.value;
            if (quoted) {
                call += '"';
            }
        }
        mErrors.emplace_back(ParserError::MissingDo(*ident, call));
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    // Parse assignment operator
    auto op = expectToken({TokenType::OP_EQUAL, TokenType::OP_PLUS_EQUAL,
                           TokenType::OP_MINUS_EQUAL, TokenType::OP_MULT_EQUAL,
                           TokenType::OP_DIVIDE_EQUAL});
    if (!op) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    switch (op->type) {
    case TokenType::OP_EQUAL: node.op = VarAssignment::ASSIGN; break;
    case TokenType::OP_PLUS_EQUAL: node.op = VarAssignment::PLUS_ASSIGN; break;
    case TokenType::OP_MINUS_EQUAL: node.op = VarAssignment::MINUS_ASSIGN; break;
    case TokenType::OP_MULT_EQUAL: node.op = VarAssignment::MULT_ASSIGN; break;
    case TokenType::OP_DIVIDE_EQUAL: node.op = VarAssignment::DIVIDE_ASSIGN; break;
    default: break;
    }

    auto value = parseExpression(tokens);
    if (!value) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }
    node.value = std::move(value.value());

    if (!expectToken(TokenType::OP_SEMICOLON)) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    return node;
}

std::optional<DoInstruction>
Parser::parseQualifiedInstruction(TokenList &tokens) {
    mTokens = &tokens;

    auto ident = expectToken(TokenType::IDENT);
    if (!ident) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    if (!expectToken(TokenType::OP_DOT)) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    auto method = expectToken(TokenType::IDENT);
    if (!method) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    auto params = parseParamExpr(tokens);
    if (!params) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    if (!expectToken(TokenType::OP_SEMICOLON)) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    DoInstruction instr;
    instr.qualifier = ident->value;
    instr.identifier = method->value;
    instr.params = std::move(params.value());
    instr.loc = locationOf(*ident);
    return instr;
}

std::optional<BreakStatement> Parser::parseBreakStatement(TokenList &tokens) {
    mTokens = &tokens;

    auto breakToken = expectToken(TokenType::K_BREAK);
    if (!breakToken) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    if (!expectToken(TokenType::OP_SEMICOLON)) {
        recoverTo(TokenType::OP_SEMICOLON);
        return std::nullopt;
    }

    return BreakStatement{locationOf(*breakToken)};
}
}

module;
#include <expected>
#include <memory>
module junopl.parser.handlers;
import junopl.lexer.tokens;
import junopl.parser.error;
import junopl.parser.nodes;
import junopl.parser.nodes.statements;

namespace JunoPL {
template <typename T> void addStatement(Body &body, T &&statement) {
    Statements stmt;
    stmt.statement = std::forward<T>(statement);
    body.body.push_back(std::make_unique<Statements>(std::move(stmt)));
}

std::expected<Body, ParserError> parseBody(TokenList &tokens) {
    auto openBrace = expectToken(tokens, TokenType::OP_CBRAC_OP);
    if (!openBrace)
        return std::unexpected(openBrace.error());

    Body body;

    if (tokens.empty()) {
        return std::unexpected(ParserError::EmptyTokenList());
    }

    while (tokens.current().type != TokenType::OP_CBRAC_CLO) {
        switch (tokens.current().type) {
        case TokenType::K_VAR: {
            auto varDecl = parseVarDeclaration(tokens);
            if (!varDecl)
                return std::unexpected(varDecl.error());
            addStatement(body, std::move(varDecl.value()));
            break;
        }
        case TokenType::K_LIST: {
            auto listDecl = parseListDeclaration(tokens);
            if (!listDecl) {
                return std::unexpected(listDecl.error());
            }
            addStatement(body, std::move(listDecl.value()));
            break;
        }
        case TokenType::K_PERFORM: {
            auto performStmt = parsePerformInstruction(tokens);
            if (!performStmt) {
                return std::unexpected(performStmt.error());
            }
            addStatement(body, std::move(performStmt.value()));
            break;
        }
        case TokenType::K_IF: {
            auto condStmt = parseConditionalStatement(tokens);
            if (!condStmt) {
                return std::unexpected(condStmt.error());
            }
            addStatement(body, std::move(condStmt.value()));
            break;
        }
        case TokenType::K_EMIT: {
            auto emitEv = parseEmitEvent(tokens);
            if (!emitEv) {
                return std::unexpected(emitEv.error());
            }
            addStatement(body, std::move(emitEv.value()));
            break;
        }
        case TokenType::K_REPEAT: {
            auto repeatLoop = parseRepeatLoop(tokens);
            if (!repeatLoop) {
                return std::unexpected(repeatLoop.error());
            }
            addStatement(body, std::move(repeatLoop.value()));
            break;
        }
        case TokenType::K_WHILE: {
            auto whileLoop = parseWhileLoop(tokens);
            if (!whileLoop) {
                return std::unexpected(whileLoop.error());
            }
            addStatement(body, std::move(whileLoop.value()));
            break;
        }
        case TokenType::K_FOR: {
            auto forLoop = parseForLoop(tokens);
            if (!forLoop) {
                return std::unexpected(forLoop.error());
            }
            addStatement(body, std::move(forLoop.value()));
            break;
        }
        default: {
            return std::unexpected(ParserError::UnexpectedToken(
                tokens.current(),
                {TokenType::K_VAR, TokenType::K_LIST, TokenType::K_PERFORM,
                 TokenType::K_IF, TokenType::K_EMIT, TokenType::K_REPEAT,
                 TokenType::K_WHILE, TokenType::K_FOR}));
        }
        }
    }

    if (tokens.current().type == TokenType::OP_CBRAC_CLO) {
        auto closeBrace = expectToken(tokens, TokenType::OP_CBRAC_CLO);
        if (!closeBrace)
            return std::unexpected(closeBrace.error());
        return body;
    }
    tokens.advance();

    switch (tokens.current().type) {
    case TokenType::OP_CBRAC_OP: {
        auto recursiveBody = parseBody(tokens);
        if (!recursiveBody)
            return std::unexpected(recursiveBody.error());
        Statements stmt;
        stmt.statement = std::move(recursiveBody.value());
        body.body.push_back(std::make_unique<Statements>(std::move(stmt)));
    }
    case TokenType::OP_CBRAC_CLO:
        return body;
    default:
        break;
    }

    return std::unexpected(ParserError::UnexpectedToken(
        tokens.current(), {TokenType::OP_CBRAC_CLO}));
}
}
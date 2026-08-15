module;
#include <memory>
#include <optional>
module junopl.parser;
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

std::optional<Body> Parser::parseBody(TokenList &tokens) {
    mTokens = &tokens;

    if (!expectToken(TokenType::OP_CBRAC_OP)) {
        recoverTo(TokenType::OP_CBRAC_CLO);
        return std::nullopt;
    }

    Body body;

    if (mTokens->empty()) {
        mErrors.emplace_back(ParserError::EmptyTokenList());
        return std::nullopt;
    }

    while (!mTokens->empty() && mTokens->current().type != TokenType::OP_CBRAC_CLO) {
        switch (mTokens->current().type) {
        case TokenType::K_VAR: {
            if (auto varDecl = parseVarDeclaration(tokens)) {
                addStatement(body, std::move(varDecl.value()));
            } else {
                recoverTo(TokenType::OP_SEMICOLON);
                if (!mTokens->empty()) mTokens->advance();
            }
            break;
        }
        case TokenType::K_LIST: {
            if (auto listDecl = parseListDeclaration(tokens)) {
                addStatement(body, std::move(listDecl.value()));
            } else {
                recoverTo(TokenType::OP_SEMICOLON);
                if (!mTokens->empty()) mTokens->advance();
            }
            break;
        }
        case TokenType::K_PERFORM: {
            if (auto performStmt = parsePerformInstruction(tokens)) {
                addStatement(body, std::move(performStmt.value()));
            } else {
                recoverTo(TokenType::OP_SEMICOLON);
                if (!mTokens->empty()) mTokens->advance();
            }
            break;
        }
        case TokenType::K_EMIT: {
            if (auto emitEv = parseEmitEvent(tokens)) {
                addStatement(body, std::move(emitEv.value()));
            } else {
                recoverTo(TokenType::OP_SEMICOLON);
                if (!mTokens->empty()) mTokens->advance();
            }
            break;
        }
        case TokenType::K_IF: {
            if (auto condStmt = parseConditionalStatement(tokens)) {
                addStatement(body, std::move(condStmt.value()));
            } else {
                recoverTo(TokenType::OP_CBRAC_CLO);
            }
            break;
        }
        case TokenType::K_REPEAT: {
            if (auto repeatLoop = parseRepeatLoop(tokens)) {
                addStatement(body, std::move(repeatLoop.value()));
            } else {
                recoverTo(TokenType::OP_CBRAC_CLO);
            }
            break;
        }
        case TokenType::K_WHILE: {
            if (auto whileLoop = parseWhileLoop(tokens)) {
                addStatement(body, std::move(whileLoop.value()));
            } else {
                recoverTo(TokenType::OP_CBRAC_CLO);
            }
            break;
        }
        case TokenType::K_FOR: {
            if (auto forLoop = parseForLoop(tokens)) {
                addStatement(body, std::move(forLoop.value()));
            } else {
                recoverTo(TokenType::OP_CBRAC_CLO);
            }
            break;
        }
        case TokenType::K_BREAK: {
            if (auto breakStmt = parseBreakStatement(tokens)) {
                addStatement(body, std::move(breakStmt.value()));
            } else {
                recoverTo(TokenType::OP_SEMICOLON);
                if (!mTokens->empty()) mTokens->advance();
            }
            break;
        }
        case TokenType::IDENT: {
            auto remaining = mTokens->remaining();
            if (remaining.size() > 1 && remaining[1].type == TokenType::OP_DOT) {
                if (auto listOp = parseListOperationStatement(tokens)) {
                    addStatement(body, std::move(listOp.value()));
                } else {
                    recoverTo(TokenType::OP_SEMICOLON);
                    if (!mTokens->empty()) mTokens->advance();
                }
            } else {
                if (auto varAssign = parseVarAssignment(tokens)) {
                    addStatement(body, std::move(varAssign.value()));
                } else {
                    recoverTo(TokenType::OP_SEMICOLON);
                    if (!mTokens->empty()) mTokens->advance();
                }
            }
            break;
        }
        default: {
            mErrors.emplace_back(ParserError::UnexpectedToken(
                mTokens->current(),
                {TokenType::K_VAR, TokenType::K_LIST, TokenType::K_PERFORM,
                 TokenType::K_IF, TokenType::K_EMIT, TokenType::K_REPEAT,
                 TokenType::K_WHILE, TokenType::K_FOR, TokenType::K_BREAK,
                 TokenType::IDENT}));
            recoverTo(TokenType::OP_SEMICOLON);
            if (!mTokens->empty()) mTokens->advance();
            break;
        }
        }
    }

    if (mTokens->empty()) {
        mErrors.emplace_back(ParserError::EmptyTokenList(TokenType::OP_CBRAC_CLO));
        return std::nullopt;
    }

    if (mTokens->current().type == TokenType::OP_CBRAC_CLO) {
        if (!expectToken(TokenType::OP_CBRAC_CLO)) {
            return std::nullopt;
        }
        return body;
    }
    mTokens->advance();

    switch (mTokens->current().type) {
    case TokenType::OP_CBRAC_OP: {
        if (auto recursiveBody = parseBody(tokens)) {
            Statements stmt;
            stmt.statement = std::move(recursiveBody.value());
            body.body.push_back(std::make_unique<Statements>(std::move(stmt)));
        } else {
            recoverTo(TokenType::OP_CBRAC_CLO);
        }
        break;
    }
    case TokenType::OP_CBRAC_CLO:
        break;
    default:
        mErrors.emplace_back(ParserError::UnexpectedToken(
            mTokens->current(), {TokenType::OP_CBRAC_CLO}));
        recoverTo(TokenType::OP_CBRAC_CLO);
        break;
    }

    return body;
}
}

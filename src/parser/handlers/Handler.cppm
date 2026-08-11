module;
#include <expected>
#include <initializer_list>
#include <string>
#include <vector>
export module junopl.parser.handlers;

import junopl.lexer.tokens;
import junopl.parser.error;
import junopl.parser.nodes;
import junopl.parser.nodes.statements;
import junopl.parser.nodes.expressions;

namespace JunoPL {
export std::expected<Token, ParserError>
expectToken(TokenList &tokens, TokenType expectedToken, bool advance = true) {
    if (tokens.empty()) {
        return std::unexpected(ParserError::EmptyTokenList(expectedToken));
    }

    Token token = tokens.current();
    if (token.type != expectedToken) {
        return std::unexpected(
            ParserError::UnexpectedToken(token, expectedToken));
    }

    if (advance) {
        tokens.advance();
    }
    return token;
}

export std::expected<Token, ParserError>
expectToken(TokenList &tokens, std::initializer_list<TokenType> expectedTokens,
            bool advance = true) {
    if (tokens.empty()) {
        return std::unexpected(ParserError::EmptyTokenList(expectedTokens));
    }

    Token token = tokens.current();
    bool found = false;
    for (TokenType expectedToken : expectedTokens) {
        if (token.type == expectedToken) {
            found = true;
        }
    }
    if (!found) {
        return std::unexpected(
            ParserError::UnexpectedToken(token, expectedTokens));
    }

    if (advance) {
        tokens.advance();
    }
    return token;
}

/**
For ordered listing */
export std::expected<std::vector<Token>, ParserError>
expectTokenList(TokenList &tokens,
                std::initializer_list<TokenType> expectedTokens) {
    std::vector<Token> toReturn;
    for (TokenType type : expectedTokens) {
        if (auto token = expectToken(tokens, type); !token) {
            return std::unexpected(token.error());
        } else {
            toReturn.push_back(token.value());
        }
    }
    return toReturn;
}

/// Root nodes
export std::expected<FunctionNode, ParserError>
parseFunction(TokenList &tokens);

export std::expected<CustomExpression, ParserError>
parseCustomExpression(TokenList &tokens);

export std::expected<EventNode, ParserError> parseEvent(TokenList &tokens);

export std::expected<VarDeclaration, ParserError>
parseVarDeclaration(TokenList &tokens);

export std::expected<ListDeclaration, ParserError>
parseListDeclaration(TokenList &tokens);

export std::expected<PerformInstruction, ParserError>
parsePerformInstruction(TokenList &tokens);

export std::expected<ConditionalStatement, ParserError>
parseConditionalStatement(TokenList &tokens);

export std::expected<RepeatLoop, ParserError>
parseRepeatLoop(TokenList &tokens);

export std::expected<WhileLoop, ParserError> parseWhileLoop(TokenList &tokens);

export std::expected<ForLoop, ParserError> parseForLoop(TokenList &tokens);

export std::expected<EmitEvent, ParserError> emitEvent(TokenList &tokens);

export std::expected<std::vector<std::string>, ParserError>
parseParam(TokenList &tokens);

export std::expected<std::vector<ExpressionHandle>, ParserError>
parseParamExpr(TokenList &tokens);

export std::expected<Body, ParserError> parseBody(TokenList &tokens);

export std::expected<ExpressionHandle, ParserError>
parseExpression(JunoPL::TokenList &tokens);

}
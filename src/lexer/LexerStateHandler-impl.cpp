module;
#include <cctype>
#include <string>
module junopl.lexer.statehandler;
import junopl.lexer.tokens.identifier;

namespace JunoPL {

LexerStateHandler::LexerStateHandler()
    : mState(States::START), mLine(0), mCol(0), isCharEOF(false) {}

void LexerStateHandler::reset() { mState = States::START; }

LexerStateHandler::Result LexerStateHandler::handle(char c, int line, int col) {
    mLine = line;
    mCol = col;

    switch (mState) {
    case States::START:
        return handleStartState(c);
    case States::IDENT:
        return handleIdentState(c);
    case States::NUMBER:
        return handleNumberState(c);
    case States::DECIMAL:
        return handleDecimalState(c);
    case States::OPERATION:
        return handleOperationState(c);
    case States::EXPECT_EQ:
        return handleExpectEqState(c);
    case States::UNKNOWN:
        return handleUnknownState(c);
    }
}

LexerStateHandler::Result LexerStateHandler::handleEOF() {
    isCharEOF = true;
    return handle(' ', -1, -1);
}

LexerStateHandler::Result LexerStateHandler::createErrorResult(char c) {
    mState = States::UNKNOWN;
    return Result::Error(LexerError{LexerError::Type::InvalidCharacter, mLine,
                                    mCol,
                                    "Invalid character: " + std::string{c}});
}

LexerStateHandler::Result LexerStateHandler::handleStartState(char c) {
    if (std::isalpha(c) || c == '_') {
        mState = States::IDENT;
        return Result::Continue();
    }
    if (std::isdigit(c)) {
        mState = States::NUMBER;
        return Result::Continue();
    }
    if (std::isspace(c)) {
        return Result::Ignore();
    }
    if (isOpSymbol(c)) {
        mState = States::OPERATION;
        return Result::Replay();
    }
    return createErrorResult(c);
}

LexerStateHandler::Result LexerStateHandler::handleIdentState(char c) {
    if (std::isalnum(c) || c == '_') {
        return Result::Continue();
    }

    if (std::isspace(c) || isOpSymbol(c)) {
        return Result::SaveReplay(TokenType::IDENT);
    }
    return createErrorResult(c);
}

LexerStateHandler::Result LexerStateHandler::handleNumberState(char c) {
    if (std::isdigit(c)) {
        return Result::Continue();
    }
    if (c == '.') {
        mState = States::DECIMAL;
        return Result::Continue();
    }
    if (std::isspace(c) || isOpSymbol(c)) {
        return Result::SaveReplay(TokenType::NUM);
    }
    return createErrorResult(c);
}

LexerStateHandler::Result LexerStateHandler::handleDecimalState(char c) {
    if (std::isdigit(c)) {
        return Result::Continue();
    }

    if (std::isspace(c) || isOpSymbol(c)) {
        return Result::SaveReplay(TokenType::NUM);
    }
    return createErrorResult(c);
}

LexerStateHandler::Result LexerStateHandler::handleOperationState(char c) {
    switch (c) {
    case '+':
    case '-':
    case '*':
    case '/':
    case '=':
    case '<':
    case '>':
        mState = States::EXPECT_EQ;
        return Result::Continue();
    case '(':
        return Result::Save(TokenType::OP_PAR_OP);
    case ')':
        return Result::Save(TokenType::OP_PAR_CLO);
    case '{':
        return Result::Save(TokenType::OP_BRAC_OP);
    case '}':
        return Result::Save(TokenType::OP_BRAC_CLO);
    case '%':
        return Result::Save(TokenType::OP_MOD);
    case '.':
        return Result::Save(TokenType::OP_DOT);
    case '?':
        return Result::Save(TokenType::OP_QUESTION);
    }
    return createErrorResult(c);
}

LexerStateHandler::Result LexerStateHandler::handleExpectEqState(char c) {
    if (c == '=') {
        return Result::InferToken();
    }
    return Result::InferTokenReplay();
}

LexerStateHandler::Result LexerStateHandler::handleUnknownState(char c) {
    // continue until next viable token
    if (std::isspace(c)) {
        reset();
    }
    return Result::Continue();
}

bool LexerStateHandler::isOpSymbol(char c) {
    switch (c) {
    case '+':
    case '-':
    case '*':
    case '/':
    case '=':
    case '(':
    case ')':
    case '{':
    case '}':
    case '<':
    case '>':
    case '%':
    case '.':
    case '?':
        return true;
    }
    return false;
}

}
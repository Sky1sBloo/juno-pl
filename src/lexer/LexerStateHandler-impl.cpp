module;
#include <cctype>
#include <print>
#include <string>
module junopl.lexer.statehandler;

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
        return Result::Continue();
    }
    return createErrorResult(c);
}

LexerStateHandler::Result LexerStateHandler::handleIdentState(char c) {
    if (std::isalnum(c) || c == '_') {
        return Result::Continue();
    }

    return Result::SaveReplay(TokenType::IDENT);
}

LexerStateHandler::Result LexerStateHandler::handleNumberState(char c) {
    if (std::isdigit(c)) {
        return Result::Continue();
    }
    if (c == '.') {
        mState = States::DECIMAL;
        return Result::Continue();
    }
    return Result::SaveReplay(TokenType::NUM);
}

LexerStateHandler::Result LexerStateHandler::handleDecimalState(char c) {
    if (std::isdigit(c)) {
        return Result::Continue();
    }
    return Result::SaveReplay(TokenType::NUM);
}

LexerStateHandler::Result LexerStateHandler::handleUnknownState(char c) {
    // continue until next viable token
    if (std::isspace(c)) {
        reset();
    }
    return Result::Continue();
}
}
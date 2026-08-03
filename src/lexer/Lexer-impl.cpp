module;
#include <stdexcept>
module junopl.lexer;
import junopl.lexer.tokens;
import junopl.lexer.tokens.identifier;

namespace JunoPL {
void Lexer::tokenize() {
    using StateResult = LexerStateHandler::Result;
    using StateAction = StateResult::Action;

    reset();
    auto c = mFileReader.read();
    if (!c.has_value()) {
        return;
    }
    while (c.has_value()) {
        StateResult result = mStateHandler.handle(c.value(), mLine, mCol);

        switch (result.action) {
        case StateAction::CONTINUE:
            mLexeme.push_back(c.value());
            break;
        case StateAction::IGNORE:
            break;
        case StateAction::REPLAY:
            continue;
        case StateAction::SAVE_TOKEN:
            mLexeme.push_back(c.value());
            saveToken(result.type);
            break;
        case StateAction::SAVE_REPLAY:
            saveToken(result.type);
            continue;
        case StateAction::SAVE_IGNORE:
            saveToken(result.type);
            break;
        case StateAction::INFER_TOKEN:
            mLexeme.push_back(c.value());
            saveToken(inferToken(mLexeme));
            break;
        case StateAction::INFER_TOKEN_REPLAY:
            saveToken(inferToken(mLexeme));
            continue;
        case StateAction::ERROR:
            if (!result.error.has_value()) {
                throw std::runtime_error(
                    "Lexer state action returns error but no error object has "
                    "been intiailized");
            }
            mStateHandler.reset();
            mErrors.push_back(result.error.value());
            saveToken(result.type);
            break;
        }

        mCol++;
        if (c.value() == '\n') {
            mLine++;
            mCol = 0;
        }
        c = mFileReader.read();
    }
    if (mLexeme.empty()) {
        return;
    }
    auto result = mStateHandler.handleEOF();
    switch (result.action) {
    case StateAction::CONTINUE:
        mErrors.push_back(LexerError{LexerError::Type::UnclosedToken, mLine,
                                     mCol, "Unclosed token on EOF"});
        break;
    case StateAction::ERROR:
        if (!result.error.has_value()) {
            throw std::runtime_error(
                "Lexer state action returns error but no error object has "
                "been intiailized");
        }
        mErrors.push_back(result.error.value());
        break;
    case StateAction::SAVE_TOKEN:
    case StateAction::SAVE_REPLAY:
        saveToken(result.type);
        break;
    default:
        break;
    }
}

void Lexer::reset() {
    mStateHandler.reset();
    mCol = 0;
    mColStart = 0;
    mLine = 0;
    mLexeme.clear();
    mTokens.clear();
}

void Lexer::saveToken(TokenType type) {
    auto keywordToken = getKeyword(mLexeme);
    TokenType tokenType = keywordToken.value_or(type);
    mTokens.push_back(Token{tokenType, mLexeme, mLine, mColStart, mCol});
    mColStart = mCol;
    mStateHandler.reset();
    mLexeme.clear();
}
}
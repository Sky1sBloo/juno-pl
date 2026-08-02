module;
#include <stdexcept>
module junopl.lexer;
import junopl.lexer.tokens;

namespace JunoPL {
void Lexer::tokenize() {
    reset();
    auto c = mFileReader.read();
    if (!c.has_value()) {
        return;
    }
    while (c.has_value()) {
        using StateResult = LexerStateHandler::Result;
        using StateAction = StateResult::Action;

        StateResult result = mStateHandler.handle(c.value(), mLine, mCol);
        mLexeme.push_back(c.value());

        switch (result.action) {
        case StateAction::CONTINUE:
            break;
        case StateAction::SAVE_TOKEN:
            mStateHandler.reset();
            saveToken(result.type);
            break;
        case StateAction::SAVE_REPLAY:
            mStateHandler.reset();
            saveToken(result.type);
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
}

void Lexer::reset() {
    mStateHandler.reset();
    mCol = 0;
    mLine = 0;
    mLexeme.clear();
    mTokens.clear();
}

void Lexer::saveToken(TokenType type) {
    mTokens.push_back(Token{type, mLexeme, mLine, mColStart, mColEnd});
    mCol = 0;
    mStateHandler.reset();
    mLexeme.clear();
}
}
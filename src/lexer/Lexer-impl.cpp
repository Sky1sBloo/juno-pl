module;
#include <stdexcept>
module junopl.lexer;
import junopl.lexer.tokens;
import junopl.lexer.tokens.identifier;

namespace JunoPL {
Lexer::Lexer() : mStateHandler(), mLine(0), mCol(0), mColStart(0), mLexeme() {}
Lexer::Output Lexer::tokenize(IFileReader &fileReader) {
    using StateResult = LexerStateHandler::Result;
    using StateAction = StateResult::Action;

    Output output;

    reset();
    auto c = fileReader.read();
    if (!c.has_value()) {
        return output;
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
            saveToken(result.type, output);
            break;
        case StateAction::SAVE_REPLAY:
            saveToken(result.type, output);
            continue;
        case StateAction::SAVE_IGNORE:
            saveToken(result.type, output);
            break;
        case StateAction::INFER_TOKEN:
            mLexeme.push_back(c.value());
            saveToken(inferToken(mLexeme), output);
            break;
        case StateAction::INFER_TOKEN_REPLAY:
            saveToken(inferToken(mLexeme), output);
            continue;
        case StateAction::ERROR:
            if (!result.error.has_value()) {
                throw std::runtime_error(
                    "Lexer state action returns error but no error object has "
                    "been intiailized");
            }
            mStateHandler.reset();
            output.errors.push_back(result.error.value());
            saveToken(result.type, output);
            break;
        }

        mCol++;
        if (c.value() == '\n') {
            mLine++;
            mCol = 0;
        }
        c = fileReader.read();
    }
    if (mLexeme.empty()) {
        return output;
    }
    auto result = mStateHandler.handleEOF();
    switch (result.action) {
    case StateAction::CONTINUE:
        output.errors.push_back(LexerError{LexerError::Type::UnclosedToken,
                                           mLine, mCol,
                                           "Unclosed token on EOF"});
        break;
    case StateAction::ERROR:
        if (!result.error.has_value()) {
            throw std::runtime_error(
                "Lexer state action returns error but no error object has "
                "been intiailized");
        }
        output.errors.push_back(result.error.value());
        break;
    case StateAction::SAVE_TOKEN:
    case StateAction::SAVE_REPLAY:
        saveToken(result.type, output);
        break;
    default:
        break;
    }

    return output;
}

void Lexer::reset() {
    mStateHandler.reset();
    mCol = 0;
    mColStart = 0;
    mLine = 0;
    mLexeme.clear();
}

void Lexer::saveToken(TokenType type, Output &output) {
    auto keywordToken = getKeyword(mLexeme);
    TokenType tokenType = keywordToken.value_or(type);
    output.tokens.emplace_back(Token{tokenType, mLexeme, mLine, mColStart, mCol});
    mColStart = mCol;
    mStateHandler.reset();
    mLexeme.clear();
}
}
module;
module junopl.lexer;
import junopl.lexer.tokens;

namespace JunoPL {
void Lexer::tokenize() {
    reset();
}

void Lexer::reset() {
    mState = LexerState::START;
    mCol = 0;
    mLine = 0;
    mLexeme.clear();
    mTokens.clear();
}

void Lexer::saveToken(TokenType type) {
    mTokens.push_back(Token{type, mLexeme, mLine, mColStart, mColEnd});
    mState = LexerState::START;
    mCol = 0;
}

}

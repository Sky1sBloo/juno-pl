module;
module junopl.lexer;
import junopl.lexer.tokens;

namespace JunoPL {
void Lexer::tokenize() {
    reset();
    auto c = mFileReader.read();
    if (!c.has_value()) {
        return;
    }
}

void Lexer::reset() {
    mCol = 0;
    mLine = 0;
    mLexeme.clear();
    mTokens.clear();
}

void Lexer::saveToken(TokenType type) {
    mTokens.push_back(Token{type, mLexeme, mLine, mColStart, mColEnd});
    mCol = 0;
}
}

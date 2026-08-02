module;
#include <string>
#include <vector>

export module junopl.lexer;
import junopl.ifilereader;
import junopl.lexer.tokens;

export namespace JunoPL {
enum class LexerState { START };

class Lexer {
  public:
    Lexer(IFileReader &fileReader)
        : mFileReader(fileReader), mState(LexerState::START) {}

    void tokenize();
    void reset();

    int test;
  private:
    void saveToken(TokenType type);

  private:
    IFileReader &mFileReader;
    LexerState mState = LexerState::START;
    std::vector<Token> mTokens;

    int mLine;
    int mCol;
    int mColStart;
    int mColEnd;
    std::string mLexeme;
    //std::vector<LexerError> mErrors;
};
}

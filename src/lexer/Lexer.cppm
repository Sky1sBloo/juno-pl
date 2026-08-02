module;
#include <string>
#include <vector>

export module junopl.lexer;
import junopl.ifilereader;
import junopl.lexer.tokens;
import junopl.lexer.statehandler;
import junopl.lexer.error;

export namespace JunoPL {

class Lexer {
  public:
    Lexer(IFileReader &fileReader)
        : mFileReader(fileReader), mStateHandler() {}

    void tokenize();
    void reset();

  private:
    void saveToken(TokenType type);

  private:
    IFileReader &mFileReader;
    LexerStateHandler mStateHandler;
    std::vector<Token> mTokens;

    int mLine;
    int mCol;
    int mColStart;
    int mColEnd;
    std::string mLexeme;
    std::vector<LexerError> mErrors;
};
}

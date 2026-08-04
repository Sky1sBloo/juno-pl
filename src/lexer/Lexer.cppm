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
    struct Output {
        std::vector<Token> tokens;
        std::vector<LexerError> errors;
    };

  public:
    Lexer();
    Output tokenize(IFileReader &fileReader);
    void reset();

  private:
    /**
    Saves the token to @param output
     */
    void saveToken(TokenType type, Output &output);

  private:
    LexerStateHandler mStateHandler;

    int mLine;
    int mCol;
    int mColStart;
    std::string mLexeme;
};
}

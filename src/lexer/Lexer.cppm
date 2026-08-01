module;
#include <expected>
#include <vector>

export module junopl.lexer;
import junopl.ifilereader;
import junopl.lexer.tokens;

namespace JunoPL {
class LexerError {
  public:
    enum class Type { FileReadError, InvalidCharacter };
    LexerError(Type type, int line, int column)
        : mType(type), mLine(line), mColumn(column) {}
    Type type() const { return mType; }
    int line() const { return mLine; }
    int column() const { return mColumn; }

  private:
    Type mType;
    int mLine;
    int mColumn;
};

enum class LexerState { START };

class Lexer {
  public:
    Lexer(IFileReader &fileReader)
        : mFileReader(fileReader), mState(LexerState::START) {}

    void reset() {}
    std::vector<Token> tokenize() {}

  private:
    IFileReader &mFileReader;
    LexerState mState = LexerState::START;

    int mLine;
    int mCol;
    int mColStart;
    int mColEnd;
    std::vector<LexerError> mErrors;
};
}

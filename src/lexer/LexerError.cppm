module;
#include <string>
export module junopl.lexer.error;

namespace JunoPL {
export class LexerError {
  public:
    enum class Type { FileReadError, InvalidCharacter };
    LexerError(Type type, int line, int column, const std::string &message)
        : mType(type), mLine(line), mColumn(column), mMessage(message) {}
    Type type() const { return mType; }
    int line() const { return mLine; }
    int column() const { return mColumn; }
    const std::string &message() const { return mMessage; }

  private:
    Type mType;
    int mLine;
    int mColumn;
    std::string mMessage;
};
}
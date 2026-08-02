export module junopl.lexer.error;

namespace JunoPL {
export class LexerError {
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
}
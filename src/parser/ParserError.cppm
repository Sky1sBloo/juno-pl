module;
#include <string>
export module junopl.parser.error;
import junopl.lexer.tokens;

namespace JunoPL {
export class ParserError {
  public:
    enum class Type { EMPTY_TOKENS, UNEXPECTED_TOKEN };
    ParserError(Type type, const Token &token, const std::string &message)
        : mType(type), mToken(token), mMessage(message) {}

    ParserError(Type type, const std::string &message)
        : mType(type), mMessage(message), mToken(Token{TokenType::UNKNOWN}) {}

    Type type() const { return mType; }
    const Token &token() const { return mToken; }
    const std::string &message() const { return mMessage; }

  private:
    Type mType;
    Token mToken;
    std::string mMessage;
};
}

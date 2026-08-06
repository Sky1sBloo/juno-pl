module;
#include <format>
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

    static ParserError UnexpectedToken(const Token &token,
                                       TokenType expectedToken) {
        return ParserError{
            Type::UNEXPECTED_TOKEN, token,
            std::format("Unexpected token: {0}, at {1}:{2}. Expected: {3}",
                        token.value, token.line, token.colStart,
                        static_cast<int>(expectedToken))}; //todo: make a string version of this
    }

  private:
    Type mType;
    Token mToken;
    std::string mMessage;
};
}

module;
#include <format>
#include <string>
export module junopl.parser.error;
import junopl.lexer.tokens;

namespace JunoPL {

// Converts a TokenType into a readable spelling for error messages.
inline std::string tokenTypeName(TokenType type) {
    switch (type) {
    case TokenType::IDENT: return "identifier";
    case TokenType::NUM: return "number";
    case TokenType::STR: return "string";
    case TokenType::TRUE: return "true";
    case TokenType::FALSE: return "false";
    case TokenType::K_PROGRAM: return "program";
    case TokenType::K_IMPORT: return "import";
    case TokenType::K_AND: return "and";
    case TokenType::K_OR: return "or";
    case TokenType::K_NOT: return "not";
    case TokenType::K_IF: return "if";
    case TokenType::K_ELIF: return "elif";
    case TokenType::K_ELSE: return "else";
    case TokenType::K_FOR: return "for";
    case TokenType::K_FROM: return "from";
    case TokenType::K_TO: return "to";
    case TokenType::K_BY: return "by";
    case TokenType::K_REPEAT: return "repeat";
    case TokenType::K_WHILE: return "while";
    case TokenType::K_BREAK: return "break";
    case TokenType::K_INSTR: return "instr";
    case TokenType::K_PERFORM: return "perform";
    case TokenType::K_ON: return "on";
    case TokenType::K_EMIT: return "emit";
    case TokenType::K_CRAFT: return "craft";
    case TokenType::K_NEARBY: return "nearby";
    case TokenType::K_EXPR: return "expr";
    case TokenType::K_VAR: return "var";
    case TokenType::K_LIST: return "list";
    case TokenType::OP_SEMICOLON: return ";";
    case TokenType::OP_COMMA: return ",";
    case TokenType::OP_PLUS: return "+";
    case TokenType::OP_MINUS: return "-";
    case TokenType::OP_MULT: return "*";
    case TokenType::OP_DIVIDE: return "/";
    case TokenType::OP_EQUAL: return "=";
    case TokenType::OP_PLUS_EQUAL: return "+=";
    case TokenType::OP_MINUS_EQUAL: return "-=";
    case TokenType::OP_MULT_EQUAL: return "*=";
    case TokenType::OP_DIVIDE_EQUAL: return "/=";
    case TokenType::OP_PAR_OP: return "(";
    case TokenType::OP_PAR_CLO: return ")";
    case TokenType::OP_BRAC_OP: return "{";
    case TokenType::OP_BRAC_CLO: return "}";
    case TokenType::OP_CBRAC_OP: return "[";
    case TokenType::OP_CBRAC_CLO: return "]";
    case TokenType::OP_MOD: return "%";
    case TokenType::OP_DOT: return ".";
    case TokenType::OP_COLON: return ":";
    case TokenType::OP_COMP_EQ: return "==";
    case TokenType::OP_COMP_NOT_EQ: return "!=";
    case TokenType::OP_COMP_GREATER: return ">";
    case TokenType::OP_COMP_LESS: return "<";
    case TokenType::OP_COMP_GREATER_EQ: return ">=";
    case TokenType::OP_COMP_LESS_EQ: return "<=";
    case TokenType::OP_QUESTION: return "?";
    case TokenType::UNKNOWN: return "unknown";
    default: return std::format("token({})", static_cast<int>(type));
    }
}
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

    static ParserError EmptyTokenList() {
        return ParserError{Type::EMPTY_TOKENS, "Parser token list is empty"};
    }

    static ParserError EmptyTokenList(TokenType expectedToken) {
        return ParserError{
            Type::EMPTY_TOKENS,
            std::format("Parser token list is empty. Expected: {0}",
                        tokenTypeName(expectedToken))};
    }

    static ParserError
    EmptyTokenList(std::initializer_list<TokenType> expectedTokens) {
        std::string expected;
        bool first = true;
        for (TokenType type : expectedTokens) {
            if (!first) {
                expected += ", ";
            }
            first = false;
            expected += tokenTypeName(type);
        }
        return ParserError{
            Type::EMPTY_TOKENS,
            std::format("Parser token list is empty. Expected: {0}", expected)};
    }

    static ParserError UnexpectedToken(const Token &token,
                                       TokenType expectedToken) {
        return ParserError{
            Type::UNEXPECTED_TOKEN, token,
            std::format(
                "Unexpected token: {0}, at {1}:{2}. Expected: {3}", token.value,
                token.line, token.colStart, tokenTypeName(expectedToken))};
    }

    static ParserError
    UnexpectedToken(const Token &token,
                    std::initializer_list<TokenType> expectedTokens) {
        std::string expected;

        bool first = true;
        for (TokenType type : expectedTokens) {
            if (!first) {
                expected += ", ";
            }
            first = false;
            expected += tokenTypeName(type);
        }

        return ParserError{
            Type::UNEXPECTED_TOKEN, token,
            std::format(
                "Unexpected token: {0}, at {1}:{2}. Expected one of: {3}",
                token.value, token.line, token.colStart, expected)};
    }

  private:
    Type mType;
    Token mToken;
    std::string mMessage;
};
}

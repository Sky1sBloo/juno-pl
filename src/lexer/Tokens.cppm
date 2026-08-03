module;
#include <string>
export module junopl.lexer.tokens;

namespace JunoPL {
export enum class TokenType {
    IDENT,
    NUM,
    STR,
    TRUE,
    FALSE,
    K_PROGRAM,
    K_IMPORT,
    OP_SEMICOLON,
    UNKNOWN
};
export struct Token {
    TokenType type;
    std::string value;
    int line;
    int colStart;
    int colEnd;
    Token(TokenType type, const std::string &value = "", int line = -1,
          int colStart = -1, int colEnd = -1)
        : type(type), value(value), line(line), colStart(colStart),
          colEnd(colEnd) {}
};

}

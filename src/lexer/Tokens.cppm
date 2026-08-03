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
    K_AND,
    K_OR,
    OP_SEMICOLON,
    OP_PLUS,
    OP_MINUS,
    OP_MULT,
    OP_DIVIDE,
    OP_EQUAL,
    OP_PAR_OP,
    OP_PAR_CLO,
    OP_BRAC_OP,
    OP_BRAC_CLO,
    OP_MOD,
    OP_DOT,
    OP_COMP_GREATER,
    OP_COMP_LESS,
    OP_COMP_GREATER_EQ,
    OP_COMP_LESS_EQ,
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

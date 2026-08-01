module;
#include <string>
export module junopl.lexer.tokens;

enum class TokenType { IDENT, NUM, STR, TRUE, FALSE, K_PROGRAM, OP_SEMICOLON };
export struct Token {
    std::string value;
    int line;
    int colStart;
    int colEnd;

    TokenType type;
};


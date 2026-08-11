module;
#include <span>
#include <string>
#include <utility>
#include <vector>
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
    K_NOT,
    K_IF,
    K_ELIF,
    K_ELSE,
    K_FOR,
    K_FROM,
    K_TO,
    K_BY,
    K_REPEAT,
    K_WHILE,
    K_BREAK,
    K_INSTR,
    K_PERFORM,
    K_ON,
    K_EMIT,
    K_CRAFT,
    K_NEARBY,
    K_EXPR,
    K_VAR,
    K_LIST,
    OP_SEMICOLON,
    OP_COMMA,
    OP_PLUS,
    OP_MINUS,
    OP_MULT,
    OP_DIVIDE,
    OP_EQUAL,
    OP_PAR_OP,
    OP_PAR_CLO,
    OP_BRAC_OP,
    OP_BRAC_CLO,
    OP_CBRAC_OP,
    OP_CBRAC_CLO,
    OP_MOD,
    OP_DOT,
    OP_COLON,
    OP_COMP_EQ,
    OP_COMP_NOT_EQ,
    OP_COMP_GREATER,
    OP_COMP_LESS,
    OP_COMP_GREATER_EQ,
    OP_COMP_LESS_EQ,
    OP_QUESTION,
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

/**
Class for handling multiple tokens for the parser
 */
export class TokenList {
  private:
    std::vector<Token> mTokens;
    std::size_t idx{0};

  public:
    void push_back(const Token &token) { mTokens.push_back(token); }
    template <typename... Args> Token &emplace_back(Args &&...args) {
        return mTokens.emplace_back(std::forward<Args>(args)...);
    }
    const Token &at(std::size_t pos) const { return mTokens.at(pos); }

    const Token &current() const { return mTokens.at(idx); }
    std::span<const Token> remaining() const {
        return {mTokens.begin() + idx, mTokens.end()};
    }
    void advance() { idx++; }

    bool empty() const { return mTokens.empty(); }
    std::size_t size() const { return mTokens.size(); }
};

}

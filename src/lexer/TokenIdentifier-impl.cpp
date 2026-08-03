module;
#include <optional>
#include <string>
#include <unordered_map>
module junopl.lexer.tokens.identifier;

namespace JunoPL {
std::optional<TokenType> getKeyword(const std::string &lexeme) {
    const auto &keyword = keywordMap.find(lexeme);
    if (keyword == keywordMap.end()) {
        return std::nullopt;
    }
    return keyword->second;
}

std::optional<TokenType> getOperator(const std::string &lexeme) {
    const auto &operatorSymbol = operatorMap.find(lexeme);
    if (operatorSymbol == operatorMap.end()) {
        return std::nullopt;
    }
    return operatorSymbol->second;
}

std::unordered_map<std::string, TokenType, string_hash, std::equal_to<>>
    keywordMap = {{"program", TokenType::K_PROGRAM},
                  {"import", TokenType::K_IMPORT},
                  {"and", TokenType::K_AND},
                  {"or", TokenType::K_OR}};

std::unordered_map<std::string, TokenType, string_hash, std::equal_to<>>
    operatorMap = {{"+", TokenType::OP_PLUS},
                   {"-", TokenType::OP_MINUS},
                   {"*", TokenType::OP_MULT},
                   {"/", TokenType::OP_DIVIDE},
                   {"=", TokenType::OP_EQUAL},
                   {"(", TokenType::OP_PAR_OP},
                   {")", TokenType::OP_PAR_CLO},
                   {"{", TokenType::OP_BRAC_OP},
                   {"}", TokenType::OP_BRAC_CLO},
                   {"%", TokenType::OP_MOD},
                   {".", TokenType::OP_DOT},
                   {">", TokenType::OP_COMP_GREATER},
                   {"<", TokenType::OP_COMP_LESS},
                   {">=", TokenType::OP_COMP_GREATER_EQ},
                   {"<=", TokenType::OP_COMP_LESS_EQ}};
}
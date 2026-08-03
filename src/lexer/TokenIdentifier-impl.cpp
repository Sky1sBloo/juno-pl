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
                  {"import", TokenType::K_IMPORT}};

std::unordered_map<std::string, TokenType, string_hash, std::equal_to<>>
    operatorMap = {{"+", TokenType::OP_PLUS},
                   {"-", TokenType::OP_MINUS},
                   {"*", TokenType::OP_MULT},
                   {"/", TokenType::OP_DIVIDE},
                   {"=", TokenType::OP_EQUAL}};
}
module;
#include <optional>
#include <string>
#include <unordered_map>
module junopl.lexer.tokens.identifier;

namespace JunoPL {
TokenType inferToken(const std::string &lexeme) {
    auto keyword = getKeyword(lexeme);
    if (keyword.has_value()) {
        return keyword.value();
    }
    keyword = getOperator(lexeme);
    if (keyword.has_value()) {
        return keyword.value();
    }
    return TokenType::UNKNOWN;
}

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
    keywordMap = {
        {"program", TokenType::K_PROGRAM}, {"import", TokenType::K_IMPORT},
        {"and", TokenType::K_AND},         {"or", TokenType::K_OR},
        {"not", TokenType::K_NOT},         {"if", TokenType::K_IF},
        {"elif", TokenType::K_ELIF},       {"else", TokenType::K_ELSE},
        {"for", TokenType::K_FOR},         {"from", TokenType::K_FROM},
        {"to", TokenType::K_TO},           {"by", TokenType::K_BY},
        {"while", TokenType::K_WHILE},     {"break", TokenType::K_BREAK},
        {"instr", TokenType::K_INSTR},     {"perform", TokenType::K_PERFORM},
        {"expr", TokenType::K_EXPR},       {"on", TokenType::K_ON},
        {"emit", TokenType::K_EMIT},       {"craft", TokenType::K_CRAFT},
        {"nearby", TokenType::K_NEARBY},   {"var", TokenType::K_VAR},
        {"list", TokenType::K_LIST},       {"true", TokenType::TRUE},
        {"false", TokenType::FALSE}};

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
                   {"==", TokenType::OP_COMP_EQ},
                   {"!=", TokenType::OP_COMP_NOT_EQ},
                   {">", TokenType::OP_COMP_GREATER},
                   {"<", TokenType::OP_COMP_LESS},
                   {">=", TokenType::OP_COMP_GREATER_EQ},
                   {"<=", TokenType::OP_COMP_LESS_EQ},
                   {"!", TokenType::K_NOT},
                   {"?", TokenType::OP_QUESTION}};
}
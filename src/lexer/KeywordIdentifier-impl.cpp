module;
#include <optional>
#include <string_view>
#include <unordered_map>
module junopl.lexer.keywordidentifier;

namespace JunoPL {
std::optional<TokenType> getKeyword(std::string_view lexeme) {
    const auto &keyword = keywordMap.find(lexeme);
    if (keyword == keywordMap.end()) {
        return std::nullopt;
    }
    return keyword->second;
}

std::unordered_map<std::string, TokenType, string_hash, std::equal_to<>>
    keywordMap = {{"program", TokenType::K_PROGRAM},
                  {"import", TokenType::K_IMPORT}};
}
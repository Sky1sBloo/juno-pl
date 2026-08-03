module;
#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>

export module junopl.lexer.tokens.identifier;
import junopl.lexer.tokens;

// for heterogeneous lookup
struct string_hash {
    using is_transparent = void;
    [[nodiscard]] std::size_t operator()(const char *txt) const {
        return std::hash<std::string_view>{}(txt);
    }

    [[nodiscard]] std::size_t operator()(std::string_view txt) const {
        return std::hash<std::string_view>{}(txt);
    }

    [[nodiscard]] std::size_t operator()(const std::string &txt) const {
        return std::hash<std::string>{}(txt);
    }
};

namespace JunoPL {
export std::optional<TokenType> getKeyword(const std::string &lexeme);

extern std::unordered_map<std::string, TokenType, string_hash, std::equal_to<>>
    keywordMap;
}
module;
#include <string>
#include <unordered_map>
export module junopl.semantic.symbols;
namespace JunoPL {
    export struct Symbol {
        enum class Kind {};

        std::string name;
        Kind kind;
        std::size_t arity;
    };

    struct Scope {
        std::unordered_map<std::string, Symbol> symbols;
    };
}
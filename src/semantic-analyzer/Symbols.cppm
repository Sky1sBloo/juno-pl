module;
#include <string>
#include <unordered_map>
export module junopl.semantic.symbols;
import junopl.common.location;
import junopl.stl;
namespace JunoPL {
export struct Symbol {
    enum class Kind { VALUE, LIST };

    std::string name;
    Kind kind;
    std::size_t arity; // for callables
    SourceLocation declaredAt;
};

export struct Scope {
    std::unordered_map<std::string, Symbol> symbols;
};
}
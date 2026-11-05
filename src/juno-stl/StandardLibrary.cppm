/**
Contains the standard library registry: every valid qualified call, with the
operations (instruction vs expression) and parameter counts each accepts.
The definitions live in StandardLibrary.cpp.
 */
module;
#include <limits>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
export module junopl.stl;

namespace JunoPL {
export class JunoSTL {
  public:
    enum class Qualifiers {
        COMMON, // No qualifier
        // General utilities
        MATH,
        STRINGS,
        INPUT,
        LISTS,
        VECTOR,
        // Game information
        GAME,
        CRAFT,
        PART,
        CAMERA,
        // Flight and navigation
        NAV,
        ALTITUDE,
        VELOCITY,
        ORBIT,
        PLANET,
        ATMOSPHERE,
        FUEL,
        TIME,
        PERFORMANCE,
        TERRAIN,
        LISTOP,
        RAYCAST,
        TARGET,
        MISC
    };

    // To identify if this is an instruction (do) or an expression
    enum class Operations { INSTRUCTION, EXPRESSION };

    /** Upper bound for variadic functions, e.g. String.join. */
    static constexpr std::size_t kUnbounded =
        std::numeric_limits<std::size_t>::max();

    /** Inclusive parameter-count range a signature accepts. */
    struct ArityRange {
        std::size_t min;
        std::size_t max;
    };

    struct Function {
        Qualifiers qualifier;
        Operations operation;
        std::string identifier;
        std::vector<ArityRange> arities;

        /** True when argCount falls inside any registered range. */
        bool accepts(std::size_t argCount) const;

        /** Human-readable summary of the accepted argument counts. */
        std::string describeArities() const;
    };

    /**
    Converts a qualifier string (e.g. "Math", "String") to a Qualifiers
    enum value. Returns COMMON when the string is empty or unknown.
     */
    static Qualifiers qualifierFromString(const std::string &qualifier);

    /**
    Converts a Qualifiers enum value back to its string form (e.g. "Math").
    Returns "Common" for Qualifiers::COMMON or any unknown value.
     */
    static std::string qualifierToString(Qualifiers qualifier);

    /**
    To find registered definition in the standard library by enum qualifier.
    Returns the first registered signature for the name (instruction or
    expression); use the operation-aware overload to pick one.
     */
    static std::optional<Function> find(Qualifiers q, const std::string &ident);

    /**
    To find registered definition in the standard library by string qualifier
     */
    static std::optional<Function> find(const std::string &qualifier,
                                        const std::string &ident);

    /**
    To find a registered definition matching both qualifier and operation type.
    Use this when the call site is known to be either an instruction (do)
    or an expression, so only functions registered for that context match.
    The same name can be registered for both contexts, e.g. Input.throttle(x)
    as an instruction and Input.throttle() as an expression.
     */
    static std::optional<Function> find(Qualifiers q, const std::string &ident,
                                        Operations op);

    /**
    String-qualifier variant of the operation-aware find.
     */
    static std::optional<Function>
    find(const std::string &qualifier, const std::string &ident, Operations op);

  private:
    /**
    Every standard-library function, keyed by (qualifier, identifier) so the
    map stays sorted for lookup. Each key maps to one entry per operation —
    the same name can be both an instruction and an expression — and each
    entry lists the parameter-count ranges it accepts (kUnbounded for
    variadic functions such as String.join).
     */
    static const std::map<std::pair<Qualifiers, std::string>,
                          std::vector<Function>>
        mRegistry;

    static const std::unordered_map<std::string, Qualifiers> kQualifierMap;

    static const std::map<Qualifiers, std::string> kQualifierReverseMap;
};
}

/**
Contains all valid library instruction calls
 */
module;
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
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
        PERFORMANCE
    };

    // To identify if its part of perform or not
    enum class Operations { INSTRUCTION, EXPRESSION };
    struct Function {
        Qualifiers qualifier;
        Operations operation;
        std::string identifier;
        std::size_t arity; // expected parameter count
    };

    /**
    Converts a qualifier string (e.g. "Math", "Strings") to a Qualifiers
    enum value. Returns COMMON when the string is empty or unknown.
     */
    static Qualifiers qualifierFromString(const std::string &qualifier) {
        auto it = kQualifierMap.find(qualifier);
        if (it != kQualifierMap.end()) {
            return it->second;
        }
        return Qualifiers::COMMON;
    }

    /**
    Converts a Qualifiers enum value back to its string form (e.g. "Math").
    Returns "Common" for Qualifiers::COMMON or any unknown value.
     */
    static std::string qualifierToString(Qualifiers qualifier) {
        auto it = kQualifierReverseMap.find(qualifier);
        if (it != kQualifierReverseMap.end()) {
            return it->second;
        }
        return "Common";
    }

    /**
    To find registered definition in the standard library by enum qualifier
     */
    static std::optional<Function> find(Qualifiers q,
                                        const std::string &ident) {
        auto it = mRegistry.find({q, ident});
        return it != mRegistry.end() ? std::optional<Function>(it->second)
                                     : std::nullopt;
    }

    /**
    To find registered definition in the standard library by string qualifier
     */
    static std::optional<Function> find(const std::string &qualifier,
                                        const std::string &ident) {
        return find(qualifierFromString(qualifier), ident);
    }

    /**
    To find a registered definition matching both qualifier and operation type.
    Use this when the call site is known to be either an instruction (perform)
    or an expression, so only functions registered for that context match.
     */
    static std::optional<Function> find(Qualifiers q, const std::string &ident,
                                        Operations op) {
        auto it = mRegistry.find({q, ident});
        if (it != mRegistry.end() && it->second.operation == op) {
            return it->second;
        }
        return std::nullopt;
    }

    /**
    String-qualifier variant of the operation-aware find.
     */
    static std::optional<Function> find(const std::string &qualifier,
                                        const std::string &ident,
                                        Operations op) {
        return find(qualifierFromString(qualifier), ident, op);
    }

  private:
    static std::map<std::pair<Qualifiers, std::string>, Function> mRegistry;

    inline static const std::unordered_map<std::string, Qualifiers>
        kQualifierMap = {{"Math", Qualifiers::MATH},
                         {"Strings", Qualifiers::STRINGS},
                         {"Input", Qualifiers::INPUT},
                         {"Lists", Qualifiers::LISTS},
                         {"Vector", Qualifiers::VECTOR},
                         {"Game", Qualifiers::GAME},
                         {"Craft", Qualifiers::CRAFT},
                         {"Part", Qualifiers::PART},
                         {"Camera", Qualifiers::CAMERA},
                         {"Nav", Qualifiers::NAV},
                         {"Altitude", Qualifiers::ALTITUDE},
                         {"Velocity", Qualifiers::VELOCITY},
                         {"Orbit", Qualifiers::ORBIT},
                         {"Planet", Qualifiers::PLANET},
                         {"Atmosphere", Qualifiers::ATMOSPHERE},
                         {"Fuel", Qualifiers::FUEL},
                         {"Time", Qualifiers::TIME},
                         {"Performance", Qualifiers::PERFORMANCE}};

    inline static const std::map<Qualifiers, std::string> kQualifierReverseMap =
        {{Qualifiers::COMMON, "Common"},
         {Qualifiers::MATH, "Math"},
         {Qualifiers::STRINGS, "Strings"},
         {Qualifiers::INPUT, "Input"},
         {Qualifiers::LISTS, "Lists"},
         {Qualifiers::VECTOR, "Vector"},
         {Qualifiers::GAME, "Game"},
         {Qualifiers::CRAFT, "Craft"},
         {Qualifiers::PART, "Part"},
         {Qualifiers::CAMERA, "Camera"},
         {Qualifiers::NAV, "Nav"},
         {Qualifiers::ALTITUDE, "Altitude"},
         {Qualifiers::VELOCITY, "Velocity"},
         {Qualifiers::ORBIT, "Orbit"},
         {Qualifiers::PLANET, "Planet"},
         {Qualifiers::ATMOSPHERE, "Atmosphere"},
         {Qualifiers::FUEL, "Fuel"},
         {Qualifiers::TIME, "Time"},
         {Qualifiers::PERFORMANCE, "Performance"}};
};
}
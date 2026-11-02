/**
Contains all valid library instruction calls
 */
module;
#include <map>
#include <string>
#include <optional>
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
    };

    /**
    Converts a qualifier string (e.g. "math", "strings") to a Qualifiers
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
    To find registered definition in the standard library by enum qualifier
     */
    static std::optional<Function> find(Qualifiers q, const std::string &ident) {
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

  private:
    static std::map<std::pair<Qualifiers, std::string>, Function> mRegistry;

    inline static const std::unordered_map<std::string, Qualifiers> kQualifierMap =
        {{"Math", Qualifiers::MATH},
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
};
}
/**
Contains all valid library instruction calls
 */
module;
#include <map>
#include <string>
#include <optional>
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
    To find registered definition in the standard library
     */
    static std::optional<Function> find(Qualifiers q, const std::string &ident) {
        auto it = mRegistry.find({q, ident});
        return it != mRegistry.end() ? std::optional<Function>(it->second) : std::nullopt;
    }

  private:
    static std::map<std::pair<Qualifiers, std::string>, Function> mRegistry;
};
}
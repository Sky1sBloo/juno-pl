/**
Contains all valid library instruction calls
 */
module;
#include <unordered_map>
#include <string>
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

  private:
};
}
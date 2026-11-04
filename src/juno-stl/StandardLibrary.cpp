/**
Definitions for the standard library registry: the arity checks, the qualifier
maps, and every registered standard-library function.
 */
module;
#include <cstddef>
#include <format>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

module junopl.stl;

namespace JunoPL {
bool JunoSTL::Function::accepts(std::size_t argCount) const {
    for (const auto &range : arities) {
        if (argCount >= range.min && argCount <= range.max) {
            return true;
        }
    }
    return false;
}

std::string JunoSTL::Function::describeArities() const {
    std::string description;
    for (std::size_t i = 0; i < arities.size(); ++i) {
        const auto &range = arities[i];
        if (i > 0) {
            description += " or ";
        }
        if (range.min == range.max) {
            description += std::format("{} argument{}", range.min,
                                       range.min == 1 ? "" : "s");
        } else if (range.max == kUnbounded) {
            description += std::format("at least {} argument{}", range.min,
                                       range.min == 1 ? "" : "s");
        } else {
            description +=
                std::format("{} to {} arguments", range.min, range.max);
        }
    }
    return description;
}

JunoSTL::Qualifiers JunoSTL::qualifierFromString(const std::string &qualifier) {
    auto it = kQualifierMap.find(qualifier);
    if (it != kQualifierMap.end()) {
        return it->second;
    }
    return Qualifiers::COMMON;
}

std::string JunoSTL::qualifierToString(JunoSTL::Qualifiers qualifier) {
    auto it = kQualifierReverseMap.find(qualifier);
    if (it != kQualifierReverseMap.end()) {
        return it->second;
    }
    return "Common";
}

std::optional<JunoSTL::Function> JunoSTL::find(JunoSTL::Qualifiers q,
                                               const std::string &ident) {
    auto it = mRegistry.find({q, ident});
    if (it == mRegistry.end() || it->second.empty()) {
        return std::nullopt;
    }
    return it->second.front();
}

std::optional<JunoSTL::Function> JunoSTL::find(const std::string &qualifier,
                                               const std::string &ident) {
    return find(qualifierFromString(qualifier), ident);
}

std::optional<JunoSTL::Function> JunoSTL::find(JunoSTL::Qualifiers q,
                                               const std::string &ident,
                                               JunoSTL::Operations op) {
    auto it = mRegistry.find({q, ident});
    if (it == mRegistry.end()) {
        return std::nullopt;
    }
    for (const auto &signature : it->second) {
        if (signature.operation == op) {
            return signature;
        }
    }
    return std::nullopt;
}

std::optional<JunoSTL::Function> JunoSTL::find(const std::string &qualifier,
                                               const std::string &ident,
                                               JunoSTL::Operations op) {
    return find(qualifierFromString(qualifier), ident, op);
}

const std::map<std::pair<JunoSTL::Qualifiers, std::string>,
               std::vector<JunoSTL::Function>>
    JunoSTL::mRegistry = [] {
        using Qualifiers = JunoSTL::Qualifiers;
        using Operations = JunoSTL::Operations;
        using ArityRange = JunoSTL::ArityRange;
        using Function = JunoSTL::Function;
        constexpr std::size_t kUnbounded = JunoSTL::kUnbounded;
        std::map<std::pair<Qualifiers, std::string>, std::vector<Function>>
            registry;
        auto add = [&registry](Qualifiers qualifier, Operations operation,
                               std::string identifier,
                               std::vector<ArityRange> arities) {
            auto key = std::pair{qualifier, identifier};
            registry[std::move(key)].push_back(Function{qualifier, operation,
                                                        std::move(identifier),
                                                        std::move(arities)});
        };

        // Program Flow - instructions
        add(Qualifiers::GAME, Operations::INSTRUCTION, "delay", {{1, 1}});
        add(Qualifiers::GAME, Operations::INSTRUCTION, "delayUntil", {{1, 1}});

        // Operators - math expressions
        add(Qualifiers::MATH, Operations::EXPRESSION, "random", {{2, 2}});
        add(Qualifiers::MATH, Operations::EXPRESSION, "min", {{2, 2}});
        add(Qualifiers::MATH, Operations::EXPRESSION, "max", {{2, 2}});
        add(Qualifiers::MATH, Operations::EXPRESSION, "not", {{1, 1}});
        add(Qualifiers::MATH, Operations::EXPRESSION, "abs", {{1, 1}});
        add(Qualifiers::MATH, Operations::EXPRESSION, "floor", {{1, 1}});
        add(Qualifiers::MATH, Operations::EXPRESSION, "ceil", {{1, 1}});
        add(Qualifiers::MATH, Operations::EXPRESSION, "round", {{1, 1}});
        add(Qualifiers::MATH, Operations::EXPRESSION, "sqrt", {{1, 1}});
        add(Qualifiers::MATH, Operations::EXPRESSION, "sin", {{1, 1}});
        add(Qualifiers::MATH, Operations::EXPRESSION, "cos", {{1, 1}});
        add(Qualifiers::MATH, Operations::EXPRESSION, "tan", {{1, 1}});
        add(Qualifiers::MATH, Operations::EXPRESSION, "asin", {{1, 1}});
        add(Qualifiers::MATH, Operations::EXPRESSION, "acos", {{1, 1}});
        add(Qualifiers::MATH, Operations::EXPRESSION, "atan", {{1, 1}});
        add(Qualifiers::MATH, Operations::EXPRESSION, "ln", {{1, 1}});
        add(Qualifiers::MATH, Operations::EXPRESSION, "log", {{1, 1}});
        add(Qualifiers::MATH, Operations::EXPRESSION, "deg2rad", {{1, 1}});
        add(Qualifiers::MATH, Operations::EXPRESSION, "rad2deg", {{1, 1}});
        add(Qualifiers::MATH, Operations::EXPRESSION, "atan2", {{2, 2}});

        // Operators - string expressions (String. and Strings. both work)
        add(Qualifiers::STRINGS, Operations::EXPRESSION, "join",
            {{2, kUnbounded}});
        add(Qualifiers::STRINGS, Operations::EXPRESSION, "lengthOf", {{1, 1}});
        add(Qualifiers::STRINGS, Operations::EXPRESSION, "letterOf", {{2, 2}});
        add(Qualifiers::STRINGS, Operations::EXPRESSION, "substr", {{3, 3}});
        add(Qualifiers::STRINGS, Operations::EXPRESSION, "strContains",
            {{2, 2}});
        add(Qualifiers::STRINGS, Operations::EXPRESSION, "format",
            {{1, kUnbounded}});
        add(Qualifiers::STRINGS, Operations::EXPRESSION, "friendly", {{2, 2}});

        // Operators - vector expressions
        add(Qualifiers::VECTOR, Operations::EXPRESSION, "vec", {{3, 3}});
        add(Qualifiers::VECTOR, Operations::EXPRESSION, "vecLength", {{1, 1}});
        add(Qualifiers::VECTOR, Operations::EXPRESSION, "vecX", {{0, 0}});
        add(Qualifiers::VECTOR, Operations::EXPRESSION, "vecY", {{0, 0}});
        add(Qualifiers::VECTOR, Operations::EXPRESSION, "vecZ", {{0, 0}});
        add(Qualifiers::VECTOR, Operations::EXPRESSION, "normalize", {{1, 1}});
        add(Qualifiers::VECTOR, Operations::EXPRESSION, "angle", {{2, 2}});
        add(Qualifiers::VECTOR, Operations::EXPRESSION, "clamp", {{2, 2}});
        add(Qualifiers::VECTOR, Operations::EXPRESSION, "cross", {{2, 2}});
        add(Qualifiers::VECTOR, Operations::EXPRESSION, "dot", {{2, 2}});
        add(Qualifiers::VECTOR, Operations::EXPRESSION, "dist", {{2, 2}});
        add(Qualifiers::VECTOR, Operations::EXPRESSION, "min", {{2, 2}});
        add(Qualifiers::VECTOR, Operations::EXPRESSION, "max", {{2, 2}});
        add(Qualifiers::VECTOR, Operations::EXPRESSION, "project", {{2, 2}});
        add(Qualifiers::VECTOR, Operations::EXPRESSION, "scale", {{2, 2}});

        // Operators - Game.funk
        add(Qualifiers::GAME, Operations::EXPRESSION, "funk", {{1, 1}});

        // Craft Instructions - input instructions
        add(Qualifiers::INPUT, Operations::INSTRUCTION, "throttle", {{1, 1}});
        add(Qualifiers::INPUT, Operations::INSTRUCTION, "pitch", {{1, 1}});
        add(Qualifiers::INPUT, Operations::INSTRUCTION, "roll", {{1, 1}});
        add(Qualifiers::INPUT, Operations::INSTRUCTION, "yaw", {{1, 1}});
        add(Qualifiers::INPUT, Operations::INSTRUCTION, "brake", {{1, 1}});
        add(Qualifiers::INPUT, Operations::INSTRUCTION, "slider", {{2, 2}});
        add(Qualifiers::INPUT, Operations::INSTRUCTION, "translateForward",
            {{1, 1}});
        add(Qualifiers::INPUT, Operations::INSTRUCTION, "translateRight",
            {{1, 1}});
        add(Qualifiers::INPUT, Operations::INSTRUCTION, "translateUp",
            {{1, 1}});
        add(Qualifiers::INPUT, Operations::INSTRUCTION, "activationGroup",
            {{2, 2}});
        add(Qualifiers::INPUT, Operations::INSTRUCTION, "translationMode",
            {{1, 1}});

        // Craft Instructions - craft state
        add(Qualifiers::CRAFT, Operations::INSTRUCTION, "pitchTo", {{1, 1}});
        add(Qualifiers::CRAFT, Operations::INSTRUCTION, "headingTo", {{1, 1}});
        add(Qualifiers::CRAFT, Operations::INSTRUCTION, "setPitchPID",
            {{3, 3}});
        add(Qualifiers::CRAFT, Operations::INSTRUCTION, "setRollPID", {{3, 3}});
        add(Qualifiers::CRAFT, Operations::INSTRUCTION, "setTarget", {{1, 1}});
        add(Qualifiers::CRAFT, Operations::INSTRUCTION, "lockPrograde",
            {{0, 0}});
        add(Qualifiers::CRAFT, Operations::INSTRUCTION, "lockRetrograde",
            {{0, 0}});
        add(Qualifiers::CRAFT, Operations::INSTRUCTION, "lockNone", {{0, 0}});
        add(Qualifiers::CRAFT, Operations::INSTRUCTION, "lockTarget", {{0, 0}});
        add(Qualifiers::CRAFT, Operations::INSTRUCTION, "lockBurnNode",
            {{0, 0}});
        add(Qualifiers::CRAFT, Operations::INSTRUCTION, "lockCurrent",
            {{0, 0}});
        add(Qualifiers::CRAFT, Operations::INSTRUCTION, "lockVector", {{3, 3}});
        add(Qualifiers::CRAFT, Operations::INSTRUCTION, "activateStage",
            {{0, 0}});

        // Craft Instructions - part control
        add(Qualifiers::PART, Operations::INSTRUCTION, "activate", {{2, 2}});
        add(Qualifiers::PART, Operations::INSTRUCTION, "focus", {{2, 2}});
        add(Qualifiers::PART, Operations::INSTRUCTION, "unfocus", {{1, 1}});
        add(Qualifiers::PART, Operations::INSTRUCTION, "name", {{2, 2}});
        add(Qualifiers::PART, Operations::INSTRUCTION, "detach", {{1, 1}});
        add(Qualifiers::PART, Operations::INSTRUCTION, "explode", {{1, 1}});
        add(Qualifiers::PART, Operations::INSTRUCTION, "drainFuel", {{1, 1}});
        add(Qualifiers::PART, Operations::INSTRUCTION, "fillFuel", {{1, 1}});

        // Craft Instructions - game actions
        add(Qualifiers::GAME, Operations::INSTRUCTION, "timeMode", {{1, 1}});
        add(Qualifiers::GAME, Operations::INSTRUCTION, "switchToCraft",
            {{1, 1}});
        add(Qualifiers::GAME, Operations::INSTRUCTION, "beep", {{3, 3}});
        add(Qualifiers::GAME, Operations::INSTRUCTION, "flightLog",
            {{1, 2}});
        add(Qualifiers::GAME, Operations::INSTRUCTION, "display", {{1, 1}});
        add(Qualifiers::GAME, Operations::EXPRESSION, "frequencyOf", {{2, 2}});
        add(Qualifiers::GAME, Operations::EXPRESSION, "speak", {{3, 3}});
        add(Qualifiers::GAME, Operations::EXPRESSION, "sineBeep", {{3, 3}});
        add(Qualifiers::GAME, Operations::EXPRESSION, "sineTriangle", {{3, 3}});
        add(Qualifiers::GAME, Operations::EXPRESSION, "sineSaw", {{3, 3}});
        add(Qualifiers::GAME, Operations::EXPRESSION, "sineSquare", {{3, 3}});
        add(Qualifiers::GAME, Operations::EXPRESSION, "sineNoise", {{3, 3}});

        // Craft Instructions - camera controls
        add(Qualifiers::CAMERA, Operations::INSTRUCTION, "zoom", {{1, 1}});
        add(Qualifiers::CAMERA, Operations::INSTRUCTION, "xRotation", {{1, 1}});
        add(Qualifiers::CAMERA, Operations::INSTRUCTION, "yRotation", {{1, 1}});
        add(Qualifiers::CAMERA, Operations::INSTRUCTION, "tilt", {{1, 1}});
        add(Qualifiers::CAMERA, Operations::INSTRUCTION, "mode", {{1, 1}});
        add(Qualifiers::CAMERA, Operations::INSTRUCTION, "targetOffset",
            {{1, 1}, {3, 3}});

        // Craft Information - expressions
        add(Qualifiers::ALTITUDE, Operations::EXPRESSION, "getAGL", {{0, 0}});
        add(Qualifiers::ALTITUDE, Operations::EXPRESSION, "getASL", {{0, 0}});
        add(Qualifiers::ALTITUDE, Operations::EXPRESSION, "getASF", {{0, 0}});

        add(Qualifiers::ORBIT, Operations::EXPRESSION, "ap", {{0, 0}});
        add(Qualifiers::ORBIT, Operations::EXPRESSION, "pe", {{0, 0}});
        add(Qualifiers::ORBIT, Operations::EXPRESSION, "timeToAp", {{0, 0}});
        add(Qualifiers::ORBIT, Operations::EXPRESSION, "timeToPe", {{0, 0}});
        add(Qualifiers::ORBIT, Operations::EXPRESSION, "eccentricity",
            {{0, 0}});
        add(Qualifiers::ORBIT, Operations::EXPRESSION, "inclination", {{0, 0}});
        add(Qualifiers::ORBIT, Operations::EXPRESSION, "period", {{0, 0}});
        add(Qualifiers::ORBIT, Operations::EXPRESSION, "planetName", {{0, 0}});

        add(Qualifiers::ATMOSPHERE, Operations::EXPRESSION, "density",
            {{0, 0}});
        add(Qualifiers::ATMOSPHERE, Operations::EXPRESSION, "pressure",
            {{0, 0}});
        add(Qualifiers::ATMOSPHERE, Operations::EXPRESSION, "speedOfSound",
            {{0, 0}});
        add(Qualifiers::ATMOSPHERE, Operations::EXPRESSION, "temperature",
            {{0, 0}});
        add(Qualifiers::ATMOSPHERE, Operations::EXPRESSION, "elements",
            {{0, 0}});
        add(Qualifiers::ATMOSPHERE, Operations::EXPRESSION, "composition",
            {{0, 0}});

        add(Qualifiers::PERFORMANCE, Operations::EXPRESSION, "engineThrust",
            {{0, 0}});
        add(Qualifiers::PERFORMANCE, Operations::EXPRESSION, "mass", {{0, 0}});
        add(Qualifiers::PERFORMANCE, Operations::EXPRESSION, "dryMass",
            {{0, 0}});
        add(Qualifiers::PERFORMANCE, Operations::EXPRESSION, "fuelMass",
            {{0, 0}});
        add(Qualifiers::PERFORMANCE, Operations::EXPRESSION, "maxEngineThrust",
            {{0, 0}});
        add(Qualifiers::PERFORMANCE, Operations::EXPRESSION, "twr", {{0, 0}});
        add(Qualifiers::PERFORMANCE, Operations::EXPRESSION, "currentISP",
            {{0, 0}});
        add(Qualifiers::PERFORMANCE, Operations::EXPRESSION, "stageDeltaV",
            {{0, 0}});
        add(Qualifiers::PERFORMANCE, Operations::EXPRESSION, "stageBurnTime",
            {{0, 0}});

        add(Qualifiers::FUEL, Operations::EXPRESSION, "battery", {{0, 0}});
        add(Qualifiers::FUEL, Operations::EXPRESSION, "stageFuel", {{0, 0}});
        add(Qualifiers::FUEL, Operations::EXPRESSION, "monoProp", {{0, 0}});
        add(Qualifiers::FUEL, Operations::EXPRESSION, "totalFuel", {{0, 0}});

        add(Qualifiers::TERRAIN, Operations::EXPRESSION, "height", {{1, 1}});
        add(Qualifiers::TERRAIN, Operations::EXPRESSION, "color", {{1, 1}});

        add(Qualifiers::LISTOP, Operations::EXPRESSION, "create", {{1, 1}});
        add(Qualifiers::LISTOP, Operations::EXPRESSION, "get", {{2, 2}});
        add(Qualifiers::LISTOP, Operations::EXPRESSION, "length", {{1, 1}});
        add(Qualifiers::LISTOP, Operations::EXPRESSION, "index", {{2, 2}});

        add(Qualifiers::NAV, Operations::EXPRESSION, "nav", {{0, 0}});
        add(Qualifiers::NAV, Operations::EXPRESSION, "target", {{0, 0}});
        add(Qualifiers::NAV, Operations::EXPRESSION, "heading", {{0, 0}});
        add(Qualifiers::NAV, Operations::EXPRESSION, "pitch", {{0, 0}});
        add(Qualifiers::NAV, Operations::EXPRESSION, "autopilotHeading",
            {{0, 0}});
        add(Qualifiers::NAV, Operations::EXPRESSION, "autopilotPitch",
            {{0, 0}});
        add(Qualifiers::NAV, Operations::EXPRESSION, "bankAngle", {{0, 0}});
        add(Qualifiers::NAV, Operations::EXPRESSION, "angleOfAttack", {{0, 0}});
        add(Qualifiers::NAV, Operations::EXPRESSION, "sideSlip", {{0, 0}});
        add(Qualifiers::NAV, Operations::EXPRESSION, "northVector", {{0, 0}});
        add(Qualifiers::NAV, Operations::EXPRESSION, "eastVector", {{0, 0}});
        add(Qualifiers::NAV, Operations::EXPRESSION, "rollAxis", {{0, 0}});
        add(Qualifiers::NAV, Operations::EXPRESSION, "pitchAxis", {{0, 0}});
        add(Qualifiers::NAV, Operations::EXPRESSION, "yawAxis", {{0, 0}});
        add(Qualifiers::NAV, Operations::EXPRESSION, "craftRight", {{0, 0}});
        add(Qualifiers::NAV, Operations::EXPRESSION, "craftUp", {{0, 0}});
        add(Qualifiers::NAV, Operations::EXPRESSION, "craftDirection",
            {{0, 0}});
        add(Qualifiers::NAV, Operations::EXPRESSION, "toLatLongAGL", {{1, 1}});
        add(Qualifiers::NAV, Operations::EXPRESSION, "toLatLongASL", {{1, 1}});
        add(Qualifiers::NAV, Operations::EXPRESSION, "toPosition", {{1, 1}});
        add(Qualifiers::NAV, Operations::EXPRESSION, "toPositionOverSea",
            {{1, 1}});
        add(Qualifiers::NAV, Operations::EXPRESSION, "getHeightAtLatLong",
            {{1, 1}});
        add(Qualifiers::NAV, Operations::EXPRESSION, "getColorAtLatLong",
            {{1, 1}});
        add(Qualifiers::NAV, Operations::EXPRESSION, "castRay", {{2, 2}});

        add(Qualifiers::VELOCITY, Operations::EXPRESSION, "orbit", {{0, 0}});
        add(Qualifiers::VELOCITY, Operations::EXPRESSION, "surface", {{0, 0}});
        add(Qualifiers::VELOCITY, Operations::EXPRESSION, "target", {{0, 0}});
        add(Qualifiers::VELOCITY, Operations::EXPRESSION, "gravity", {{0, 0}});
        add(Qualifiers::VELOCITY, Operations::EXPRESSION, "drag", {{0, 0}});
        add(Qualifiers::VELOCITY, Operations::EXPRESSION, "acceleration",
            {{0, 0}});
        add(Qualifiers::VELOCITY, Operations::EXPRESSION, "angular", {{0, 0}});
        add(Qualifiers::VELOCITY, Operations::EXPRESSION, "lateral", {{0, 0}});
        add(Qualifiers::VELOCITY, Operations::EXPRESSION, "vertical", {{0, 0}});
        add(Qualifiers::VELOCITY, Operations::EXPRESSION, "mach", {{0, 0}});

        add(Qualifiers::TARGET, Operations::EXPRESSION, "name", {{0, 0}});
        add(Qualifiers::TARGET, Operations::EXPRESSION, "planet", {{0, 0}});

        add(Qualifiers::INPUT, Operations::EXPRESSION, "throttle", {{0, 0}});
        add(Qualifiers::INPUT, Operations::EXPRESSION, "pitch", {{0, 0}});
        add(Qualifiers::INPUT, Operations::EXPRESSION, "roll", {{0, 0}});
        add(Qualifiers::INPUT, Operations::EXPRESSION, "yaw", {{0, 0}});
        add(Qualifiers::INPUT, Operations::EXPRESSION, "brake", {{0, 0}});
        add(Qualifiers::INPUT, Operations::EXPRESSION, "slider", {{1, 1}});
        add(Qualifiers::INPUT, Operations::EXPRESSION, "slider2", {{0, 0}});
        add(Qualifiers::INPUT, Operations::EXPRESSION, "slider3", {{0, 0}});
        add(Qualifiers::INPUT, Operations::EXPRESSION, "slider4", {{0, 0}});
        add(Qualifiers::INPUT, Operations::EXPRESSION, "translateForward",
            {{0, 0}});
        add(Qualifiers::INPUT, Operations::EXPRESSION, "translateRight",
            {{0, 0}});
        add(Qualifiers::INPUT, Operations::EXPRESSION, "translateUp", {{0, 0}});
        add(Qualifiers::INPUT, Operations::EXPRESSION, "translationMode",
            {{0, 0}});
        add(Qualifiers::INPUT, Operations::EXPRESSION, "activationGroup",
            {{1, 1}});

        add(Qualifiers::CRAFT, Operations::EXPRESSION, "name",
            {{0, 0}, {1, 1}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "idOf", {{1, 1}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "currentStage",
            {{0, 0}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "numStages", {{0, 0}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "isGrounded",
            {{0, 0}, {1, 1}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "solarRadiation",
            {{0, 0}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "pitchPID", {{0, 0}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "rollPID", {{0, 0}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "targetName", {{0, 0}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "targetPlanet",
            {{0, 0}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "idOf", {{1, 1}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "position", {{1, 1}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "velocity", {{1, 1}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "isPlayer", {{1, 1}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "altitude", {{1, 1}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "isDestroyed", {{1, 1}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "mass", {{1, 1}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "partCount", {{1, 1}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "planet", {{1, 1}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "boundingBoxMin",
            {{1, 1}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "boundingBoxMax",
            {{1, 1}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "ap", {{1, 1}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "pe", {{1, 1}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "orbitAp", {{1, 1}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "orbitPe", {{1, 1}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "period", {{1, 1}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "period", {{1, 1}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "orbitPeriod", {{1, 1}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "apoapsisTime",
            {{1, 1}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "periapsisTime",
            {{1, 1}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "inclination",
            {{1, 1}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "eccentricity",
            {{1, 1}});

        add(Qualifiers::CAMERA, Operations::EXPRESSION, "position", {{0, 0}});
        add(Qualifiers::CAMERA, Operations::EXPRESSION, "pointing", {{0, 0}});
        add(Qualifiers::CAMERA, Operations::EXPRESSION, "upDirection", {{0, 0}});
        add(Qualifiers::CAMERA, Operations::EXPRESSION, "mode", {{0, 0}});
        add(Qualifiers::CAMERA, Operations::EXPRESSION, "modeIndex", {{0, 0}});
        add(Qualifiers::CAMERA, Operations::EXPRESSION, "rotationX", {{0, 0}});
        add(Qualifiers::CAMERA, Operations::EXPRESSION, "rotationY", {{0, 0}});
        add(Qualifiers::CAMERA, Operations::EXPRESSION, "tilt", {{0, 0}});
        add(Qualifiers::CAMERA, Operations::EXPRESSION, "zoom", {{0, 0}});
        add(Qualifiers::CAMERA, Operations::EXPRESSION, "soundFrequency",
            {{2, 2}});
        add(Qualifiers::CAMERA, Operations::EXPRESSION, "targetOffset",
            {{0, 0}});

        add(Qualifiers::MISC, Operations::EXPRESSION, "grounded", {{0, 0}});
        add(Qualifiers::MISC, Operations::EXPRESSION, "stage", {{0, 0}});
        add(Qualifiers::MISC, Operations::EXPRESSION, "numStages", {{0, 0}});
        add(Qualifiers::MISC, Operations::EXPRESSION, "solarRadiation",
            {{0, 0}});
        add(Qualifiers::MISC, Operations::EXPRESSION, "cameraPosition",
            {{0, 0}});
        add(Qualifiers::MISC, Operations::EXPRESSION, "cameraPointing",
            {{0, 0}});
        add(Qualifiers::MISC, Operations::EXPRESSION, "cameraUp", {{0, 0}});
        add(Qualifiers::MISC, Operations::EXPRESSION, "pidPitch", {{0, 0}});
        add(Qualifiers::MISC, Operations::EXPRESSION, "pidRoll", {{0, 0}});

        add(Qualifiers::TIME, Operations::EXPRESSION, "sinceLaunch", {{0, 0}});
        add(Qualifiers::TIME, Operations::EXPRESSION, "delta", {{0, 0}});
        add(Qualifiers::TIME, Operations::EXPRESSION, "total", {{0, 0}});
        add(Qualifiers::TIME, Operations::EXPRESSION, "warpAmount", {{0, 0}});
        add(Qualifiers::TIME, Operations::EXPRESSION, "realTime", {{0, 0}});

        add(Qualifiers::PLANET, Operations::EXPRESSION, "radius", {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "mass", {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "solidGround",
            {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "surfaceAirDensity",
            {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "atmosphereHeight",
            {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "atmosphereScaleHeight",
            {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "atmosphereElements",
            {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION,
            "atmosphereComposition", {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "soiRadius",
            {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "solarPosition",
            {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "childPlanets",
            {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "crafts", {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "craftID", {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "parent", {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "structures", {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "hydrosphereElement",
            {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION,
            "hydrosphereComposition", {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "hydrosphereDensity",
            {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "dayLength", {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "yearLength", {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "velocity", {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "orbitAp", {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "orbitPe", {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "orbitPeriod",
            {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "orbitApTime",
            {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "orbitPeTime",
            {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "orbitInclination",
            {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "orbitEccentricity",
            {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "orbitMeanAnomaly",
            {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "orbitMeanMotion",
            {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION,
            "orbitPeriapsisArgument", {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION,
            "orbitRightAscension", {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION,
            "orbitTrueAnomaly", {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION,
            "orbitSemiMajorAxis", {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "orbitSemiMinorAxis",
            {{1, 1}});

        add(Qualifiers::PLANET, Operations::EXPRESSION, "hasTerrain",
            {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION,
            "atmosphereDensity", {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION,
            "atmosphereScale", {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "soiradius",
            {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "day", {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "year", {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "apoapsis",
            {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "periapsis",
            {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "period", {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "inclination",
            {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "eccentricity",
            {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "meananomaly",
            {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "meanmotion",
            {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "semimajoraxis",
            {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "semiminoraxis",
            {{1, 1}});

        add(Qualifiers::CRAFT, Operations::EXPRESSION, "boundMin",
            {{1, 1}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "boundMax",
            {{1, 1}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "meanAnomaly",
            {{1, 1}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "meanMotion",
            {{1, 1}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "periapsisArgument",
            {{1, 1}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "rightAscension",
            {{1, 1}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "trueAnomaly",
            {{1, 1}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "semiMajorAxis",
            {{1, 1}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "semiMinorAxis",
            {{1, 1}});
        add(Qualifiers::CRAFT, Operations::EXPRESSION, "nameToID",
            {{1, 1}});

        add(Qualifiers::PART, Operations::EXPRESSION, "thisID", {{1, 1}});
        add(Qualifiers::PART, Operations::EXPRESSION, "nameToID",
            {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "orbitMeanMotion",
            {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION,
            "orbitPeriapsisArgument", {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "orbitRightAscension",
            {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "orbitTrueAnomaly",
            {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "orbitSemiMajorAxis",
            {{1, 1}});
        add(Qualifiers::PLANET, Operations::EXPRESSION, "orbitSemiMinorAxis",
            {{1, 1}});

        add(Qualifiers::RAYCAST, Operations::EXPRESSION, "raycast", {{2, 2}});

        add(Qualifiers::PART, Operations::EXPRESSION, "idOf", {{1, 1}});
        add(Qualifiers::PART, Operations::EXPRESSION, "activated", {{1, 1}});
        add(Qualifiers::PART, Operations::EXPRESSION, "name", {{1, 1}});
        add(Qualifiers::PART, Operations::EXPRESSION, "mass", {{1, 1}});
        add(Qualifiers::PART, Operations::EXPRESSION, "dryMass", {{1, 1}});
        add(Qualifiers::PART, Operations::EXPRESSION, "wetMass", {{1, 1}});
        add(Qualifiers::PART, Operations::EXPRESSION, "partType", {{1, 1}});
        add(Qualifiers::PART, Operations::EXPRESSION, "position", {{1, 1}});
        add(Qualifiers::PART, Operations::EXPRESSION, "temperature", {{1, 1}});
        add(Qualifiers::PART, Operations::EXPRESSION, "drag", {{1, 1}});
        add(Qualifiers::PART, Operations::EXPRESSION, "fuelTransfer",
            {{1, 1}});
        add(Qualifiers::PART, Operations::EXPRESSION, "idToName", {{1, 1}});
        add(Qualifiers::PART, Operations::EXPRESSION, "programPartId",
            {{1, 1}});
        add(Qualifiers::PART, Operations::EXPRESSION, "minPartId", {{1, 1}});
        add(Qualifiers::PART, Operations::EXPRESSION, "maxPartId", {{1, 1}});
        add(Qualifiers::PART, Operations::EXPRESSION, "underwater", {{1, 1}});
        add(Qualifiers::PART, Operations::EXPRESSION, "localToPCI", {{2, 2}});
        add(Qualifiers::PART, Operations::EXPRESSION, "pciToLocal", {{2, 2}});

        // Events - variables
        add(Qualifiers::INPUT, Operations::INSTRUCTION, "setVariable",
            {{2, 2}});

        // Lists - qualified form only: Lists.insert(ident, value, ...)
        add(Qualifiers::LISTS, Operations::INSTRUCTION, "insert",
            {{2, kUnbounded}});
        add(Qualifiers::LISTS, Operations::INSTRUCTION, "set", {{3, 3}});
        add(Qualifiers::LISTS, Operations::INSTRUCTION, "remove", {{2, 2}});
        add(Qualifiers::LISTS, Operations::INSTRUCTION, "clear", {{1, 1}});
        add(Qualifiers::LISTS, Operations::INSTRUCTION, "sort", {{1, 1}});
        add(Qualifiers::LISTS, Operations::INSTRUCTION, "reverse", {{1, 1}});
        add(Qualifiers::LISTS, Operations::EXPRESSION, "length", {{1, 1}});
        add(Qualifiers::LISTS, Operations::EXPRESSION, "idxOf", {{2, 2}});
        add(Qualifiers::LISTS, Operations::EXPRESSION, "index", {{2, 2}});

        return registry;
    }();

const std::unordered_map<std::string, JunoSTL::Qualifiers>
    JunoSTL::kQualifierMap = {
        {"Math", JunoSTL::Qualifiers::MATH},
        {"String", JunoSTL::Qualifiers::STRINGS},
        {"Strings", JunoSTL::Qualifiers::STRINGS},
        {"Input", JunoSTL::Qualifiers::INPUT},
        {"Lists", JunoSTL::Qualifiers::LISTS},
        {"ListOp", JunoSTL::Qualifiers::LISTOP},
        {"Vector", JunoSTL::Qualifiers::VECTOR},
        {"Game", JunoSTL::Qualifiers::GAME},
        {"Craft", JunoSTL::Qualifiers::CRAFT},
        {"Part", JunoSTL::Qualifiers::PART},
        {"Terrain", JunoSTL::Qualifiers::TERRAIN},
        {"Camera", JunoSTL::Qualifiers::CAMERA},
        {"Nav", JunoSTL::Qualifiers::NAV},
        {"Altitude", JunoSTL::Qualifiers::ALTITUDE},
        {"Velocity", JunoSTL::Qualifiers::VELOCITY},
        {"Orbit", JunoSTL::Qualifiers::ORBIT},
        {"Planet", JunoSTL::Qualifiers::PLANET},
        {"Atmosphere", JunoSTL::Qualifiers::ATMOSPHERE},
        {"Fuel", JunoSTL::Qualifiers::FUEL},
        {"Time", JunoSTL::Qualifiers::TIME},
        {"Performance", JunoSTL::Qualifiers::PERFORMANCE},
        {"Terrain", JunoSTL::Qualifiers::TERRAIN},
        {"ListOp", JunoSTL::Qualifiers::LISTOP},
        {"Raycast", JunoSTL::Qualifiers::RAYCAST},
        {"Target", JunoSTL::Qualifiers::TARGET},
        {"Misc", JunoSTL::Qualifiers::MISC}};

const std::map<JunoSTL::Qualifiers, std::string> JunoSTL::kQualifierReverseMap =
    {{JunoSTL::Qualifiers::COMMON, "Common"},
     {JunoSTL::Qualifiers::MATH, "Math"},
     {JunoSTL::Qualifiers::STRINGS, "Strings"},
     {JunoSTL::Qualifiers::INPUT, "Input"},
     {JunoSTL::Qualifiers::LISTS, "Lists"},
     {JunoSTL::Qualifiers::VECTOR, "Vector"},
     {JunoSTL::Qualifiers::GAME, "Game"},
     {JunoSTL::Qualifiers::CRAFT, "Craft"},
     {JunoSTL::Qualifiers::PART, "Part"},
     {JunoSTL::Qualifiers::CAMERA, "Camera"},
     {JunoSTL::Qualifiers::NAV, "Nav"},
     {JunoSTL::Qualifiers::ALTITUDE, "Altitude"},
     {JunoSTL::Qualifiers::VELOCITY, "Velocity"},
     {JunoSTL::Qualifiers::ORBIT, "Orbit"},
     {JunoSTL::Qualifiers::PLANET, "Planet"},
     {JunoSTL::Qualifiers::ATMOSPHERE, "Atmosphere"},
     {JunoSTL::Qualifiers::FUEL, "Fuel"},
     {JunoSTL::Qualifiers::TIME, "Time"},
     {JunoSTL::Qualifiers::PERFORMANCE, "Performance"},
     {JunoSTL::Qualifiers::TERRAIN, "Terrain"},
     {JunoSTL::Qualifiers::LISTOP, "ListOp"},
     {JunoSTL::Qualifiers::RAYCAST, "Raycast"},
     {JunoSTL::Qualifiers::TARGET, "Target"},
     {JunoSTL::Qualifiers::MISC, "Misc"}};
}

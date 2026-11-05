module;
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <format>
#include <functional>
#include <map>
#include <pugixml.hpp>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>
module junopl.compiler;

/**
For visitors
*/
template <class... Ts> struct Overload : Ts... {
    using Ts::operator()...;
};

namespace JunoPL {

namespace {

const char *binopOp(BinaryOp::Operation o) {
    switch (o) {
    case BinaryOp::Operation::ADD:
        return "+";
    case BinaryOp::Operation::SUBTRUCT:
        return "-";
    case BinaryOp::Operation::MULTIPLY:
        return "*";
    case BinaryOp::Operation::DIVIDE:
        return "/";
    case BinaryOp::Operation::MOD:
        return "%";
    case BinaryOp::Operation::AND:
        return "and";
    case BinaryOp::Operation::OR:
        return "or";
    case BinaryOp::Operation::EQUAL:
        return "=";
    case BinaryOp::Operation::NOT_EQUAL:
        return "!=";
    case BinaryOp::Operation::GREATER:
        return ">";
    case BinaryOp::Operation::LESS:
        return "<";
    case BinaryOp::Operation::GREATER_EQ:
        return ">=";
    case BinaryOp::Operation::LESS_EQ:
        return "<=";
    }
    return "";
}

const char *binopStyle(BinaryOp::Operation o) {
    switch (o) {
    case BinaryOp::Operation::ADD:
        return "op-add";
    case BinaryOp::Operation::SUBTRUCT:
        return "op-sub";
    case BinaryOp::Operation::MULTIPLY:
        return "op-mul";
    case BinaryOp::Operation::DIVIDE:
        return "op-div";
    case BinaryOp::Operation::MOD:
        return "op-mod";
    case BinaryOp::Operation::AND:
        return "op-and";
    case BinaryOp::Operation::OR:
        return "op-or";
    case BinaryOp::Operation::EQUAL:
        return "op-eq";
    case BinaryOp::Operation::NOT_EQUAL:
        // No program contains a not-equal style: != is emitted as a Not
        // block around an equality comparison, which uses op-eq
        return "op-eq";
    case BinaryOp::Operation::GREATER:
        return "op-gt";
    case BinaryOp::Operation::LESS:
        return "op-lt";
    case BinaryOp::Operation::GREATER_EQ:
        return "op-gte";
    case BinaryOp::Operation::LESS_EQ:
        return "op-lte";
    }
    return "";
}

/**
Vizzy comparison blocks store their operator as a short code, not the source
spelling: "=" for equality, "g"/"l"/"ge"/"le" for the ordered ones. There is
no "not equal" code anywhere in the reference programs, so != is emitted as
Not(=) instead of an operator the game may not know.
 */
const char *comparisonOp(BinaryOp::Operation o) {
    switch (o) {
    case BinaryOp::Operation::EQUAL:
    case BinaryOp::Operation::NOT_EQUAL:
        return "=";
    case BinaryOp::Operation::GREATER:
        return "g";
    case BinaryOp::Operation::LESS:
        return "l";
    case BinaryOp::Operation::GREATER_EQ:
        return "ge";
    case BinaryOp::Operation::LESS_EQ:
        return "le";
    default:
        return "";
    }
}

bool isComparisonOp(BinaryOp::Operation o) {
    switch (o) {
    case BinaryOp::Operation::EQUAL:
    case BinaryOp::Operation::NOT_EQUAL:
    case BinaryOp::Operation::GREATER:
    case BinaryOp::Operation::LESS:
    case BinaryOp::Operation::GREATER_EQ:
    case BinaryOp::Operation::LESS_EQ:
        return true;
    default:
        return false;
    }
}

/**
Standard library expression calls (e.g. Orbit.ap()) compile to Vizzy
CraftProperty blocks, not CallCustomExpression. This maps qualifier+identifier
to the Vizzy property name and style block.
*/
struct StdExpr {
    const char *property;
    const char *style;
};

const StdExpr *findStdExpr(const std::string &qualifier,
                           const char *identifier) {
    static const std::map<std::pair<std::string, std::string>, StdExpr>
        kStdExprs = {
            // Orbit expressions
            {{"Orbit", "ap"}, {"Orbit.Apoapsis", "prop-orbit"}},
            {{"Orbit", "pe"}, {"Orbit.Periapsis", "prop-orbit"}},
            {{"Orbit", "timeToAp"}, {"Orbit.TimeToApoapsis", "prop-orbit"}},
            {{"Orbit", "timeToPe"}, {"Orbit.TimeToPeriapsis", "prop-orbit"}},
            {{"Orbit", "eccentricity"}, {"Orbit.Eccentricity", "prop-orbit"}},
            {{"Orbit", "inclination"}, {"Orbit.Inclination", "prop-orbit"}},
            {{"Orbit", "period"}, {"Orbit.Period", "prop-orbit"}},
            {{"Orbit", "planetName"}, {"Orbit.Planet", "prop-name"}},
            // Altitude expressions
            {{"Altitude", "getAGL"}, {"Altitude.AGL", "prop-altitude"}},
            {{"Altitude", "getASL"}, {"Altitude.ASL", "prop-altitude"}},
            {{"Altitude", "getASF"}, {"Altitude.ASF", "prop-altitude"}},
            // Atmosphere expressions
            {{"Atmosphere", "density"}, {"Atmosphere.AirDensity", "prop-atmosphere"}},
            {{"Atmosphere", "pressure"}, {"Atmosphere.AirPressure", "prop-atmosphere"}},
            {{"Atmosphere", "speedOfSound"}, {"Atmosphere.SpeedOfSound", "prop-atmosphere"}},
            {{"Atmosphere", "temperature"}, {"Atmosphere.Temperature", "prop-atmosphere"}},
            {{"Atmosphere", "elements"}, {"Atmosphere.Elements", "prop-atmosphere"}},
            {{"Atmosphere", "composition"}, {"Atmosphere.Composition", "prop-atmosphere"}},
            // Performance expressions
            {{"Performance", "engineThrust"}, {"Performance.CurrentEngineThrust", "prop-performance"}},
            {{"Performance", "mass"}, {"Performance.Mass", "prop-performance"}},
            {{"Performance", "dryMass"}, {"Performance.DryMass", "prop-performance"}},
            {{"Performance", "fuelMass"}, {"Performance.FuelMass", "prop-performance"}},
            {{"Performance", "maxEngineThrust"}, {"Performance.MaxActiveEngineThrust", "prop-performance"}},
            {{"Performance", "twr"}, {"Performance.TWR", "prop-performance"}},
            {{"Performance", "currentISP"}, {"Performance.CurrentIsp", "prop-performance"}},
            {{"Performance", "stageDeltaV"}, {"Performance.StageDeltaV", "prop-performance"}},
            {{"Performance", "stageBurnTime"}, {"Performance.BurnTime", "prop-performance"}},
            // Fuel expressions
            {{"Fuel", "battery"}, {"Fuel.Battery", "prop-fuel"}},
            {{"Fuel", "stageFuel"}, {"Fuel.FuelInStage", "prop-fuel"}},
            {{"Fuel", "monoProp"}, {"Fuel.Mono", "prop-fuel"}},
            {{"Fuel", "totalFuel"}, {"Fuel.AllStages", "prop-fuel"}},
            // Nav expressions
            {{"Nav", "nav"}, {"Nav.Position", "prop-nav"}},
            {{"Nav", "target"}, {"Target.Position", "prop-nav"}},
            {{"Nav", "heading"}, {"Nav.CraftHeading", "prop-nav"}},
            {{"Nav", "pitch"}, {"Nav.Pitch", "prop-nav"}},
            {{"Nav", "autopilotHeading"}, {"Nav.AutopilotHeading", "prop-nav"}},
            {{"Nav", "autopilotPitch"}, {"Nav.AutopilotPitch", "prop-nav"}},
            {{"Nav", "bankAngle"}, {"Nav.BankAngle", "prop-nav"}},
            {{"Nav", "angleOfAttack"}, {"Nav.AngleOfAttack", "prop-nav"}},
            {{"Nav", "sideSlip"}, {"Nav.SideSlip", "prop-nav"}},
            {{"Nav", "northVector"}, {"Nav.North", "prop-nav"}},
            {{"Nav", "eastVector"}, {"Nav.East", "prop-nav"}},
            {{"Nav", "rollAxis"}, {"Nav.CraftDirection", "prop-nav"}},
            {{"Nav", "pitchAxis"}, {"Nav.CraftRight", "prop-nav"}},
            {{"Nav", "yawAxis"}, {"Nav.CraftUp", "prop-nav"}},
            {{"Nav", "craftRight"}, {"Nav.CraftRight", "prop-nav"}},
            {{"Nav", "craftUp"}, {"Nav.CraftUp", "prop-nav"}},
            // Velocity expressions
            {{"Velocity", "orbit"}, {"Vel.OrbitVelocity", "prop-velocity"}},
            {{"Velocity", "surface"}, {"Vel.SurfaceVelocity", "prop-velocity"}},
            {{"Velocity", "target"}, {"Target.Velocity", "prop-velocity"}},
            {{"Velocity", "gravity"}, {"Vel.Gravity", "prop-velocity"}},
            {{"Velocity", "drag"}, {"Vel.Drag", "prop-velocity"}},
            {{"Velocity", "acceleration"}, {"Vel.Acceleration", "prop-velocity"}},
            {{"Velocity", "angular"}, {"Vel.AngularVelocity", "prop-velocity"}},
            {{"Velocity", "lateral"}, {"Vel.LateralSurfaceVelocity", "prop-velocity"}},
            {{"Velocity", "vertical"}, {"Vel.VerticalSurfaceVelocity", "prop-velocity"}},
            {{"Velocity", "mach"}, {"Vel.MachNumber", "prop-velocity"}},
            // Input expressions
            {{"Input", "throttle"}, {"Input.Throttle", "prop-input"}},
            {{"Input", "pitch"}, {"Input.Pitch", "prop-input"}},
            {{"Input", "roll"}, {"Input.Roll", "prop-input"}},
            {{"Input", "yaw"}, {"Input.Yaw", "prop-input"}},
            {{"Input", "brake"}, {"Input.Brake", "prop-input"}},
            {{"Input", "slider"}, {"Input.Slider1", "prop-input"}},
            {{"Input", "translateForward"}, {"Input.TranslateForward", "prop-input"}},
            {{"Input", "translateRight"}, {"Input.TranslateRight", "prop-input"}},
            {{"Input", "translateUp"}, {"Input.TranslateUp", "prop-input"}},
            {{"Input", "translationMode"}, {"Input.TranslationMode", "prop-input"}},
            {{"Input", "activationGroup"}, {"Input.ActivationGroup", "prop-input"}},
            // Camera expressions
            {{"Camera", "position"}, {"Camera.Position", "prop-camera"}},
            {{"Camera", "pointing"}, {"Camera.Pointing", "prop-camera"}},
            {{"Camera", "upDirection"}, {"Camera.UpDirection", "prop-camera"}},
            {{"Camera", "mode"}, {"Camera.Mode", "prop-camera"}},
            {{"Camera", "modeIndex"}, {"Camera.ModeIndex", "prop-camera"}},
            {{"Camera", "rotationX"}, {"Camera.RotationX", "prop-camera"}},
            {{"Camera", "rotationY"}, {"Camera.RotationY", "prop-camera"}},
            {{"Camera", "tilt"}, {"Camera.Tilt", "prop-camera"}},
            {{"Camera", "zoom"}, {"Camera.Zoom", "prop-camera"}},
            {{"Camera", "soundFrequency"}, {"Sound.Frequency", "note-frequency"}},
            {{"Camera", "targetOffset"}, {"Camera.TargetOffset", "prop-camera"}},
            // Misc expressions
            {{"Misc", "grounded"}, {"Misc.Grounded", "prop-misc"}},
            {{"Misc", "stage"}, {"Misc.Stage", "prop-misc"}},
            {{"Misc", "numStages"}, {"Misc.NumStages", "prop-misc"}},
            {{"Misc", "solarRadiation"}, {"Misc.SolarRadiation", "prop-misc"}},
            {{"Misc", "cameraPosition"}, {"Misc.CameraPosition", "prop-misc"}},
            {{"Misc", "cameraPointing"}, {"Misc.CameraPointing", "prop-misc"}},
            {{"Misc", "cameraUp"}, {"Misc.CameraUp", "prop-misc"}},
            {{"Misc", "pidPitch"}, {"Misc.PidPitch", "prop-misc"}},
            {{"Misc", "pidRoll"}, {"Misc.PidRoll", "prop-misc"}},
            // Time expressions
            {{"Time", "sinceLaunch"}, {"Time.TimeSinceLaunch", "prop-time"}},
            {{"Time", "delta"}, {"Time.FrameDeltaTime", "prop-time"}},
            {{"Time", "total"}, {"Time.TotalTime", "prop-time"}},
            {{"Time", "warpAmount"}, {"Time.WarpAmount", "prop-time"}},
            {{"Time", "realTime"}, {"Time.RealTime", "prop-time"}},
            // Craft expressions
            {{"Craft", "ap"}, {"Craft.Apoapsis", "craft"}},
            {{"Craft", "orbitAp"}, {"Craft.Apoapsis", "craft"}},
            {{"Craft", "pe"}, {"Craft.Periapsis", "craft"}},
            {{"Craft", "orbitPe"}, {"Craft.Periapsis", "craft"}},
            {{"Craft", "timeToAp"}, {"Craft.ApoapsisTime", "craft"}},
            {{"Craft", "timeToPe"}, {"Craft.PeriapsisTime", "craft"}},
            {{"Craft", "eccentricity"}, {"Craft.Eccentricity", "craft"}},
            {{"Craft", "inclination"}, {"Craft.Inclination", "craft"}},
            {{"Craft", "period"}, {"Craft.Period", "craft"}},
            {{"Craft", "orbitPeriod"}, {"Craft.Period", "craft"}},
            {{"Craft", "planet"}, {"Craft.Planet", "craft"}},
            {{"Craft", "planetName"}, {"Craft.Planet", "craft"}},
            {{"Craft", "mass"}, {"Craft.Mass", "craft"}},
            {{"Craft", "altitude"}, {"Craft.Altitude", "craft"}},
            {{"Craft", "isDestroyed"}, {"Craft.Destroyed", "craft"}},
            {{"Craft", "isGrounded"}, {"Craft.Grounded", "craft"}},
            {{"Craft", "partCount"}, {"Craft.PartCount", "craft"}},
            {{"Craft", "position"}, {"Craft.Position", "craft"}},
            {{"Craft", "velocity"}, {"Craft.Velocity", "craft"}},
            {{"Craft", "isPlayer"}, {"Craft.IsPlayer", "craft"}},
            {{"Craft", "boundingBoxMin"}, {"Craft.BoundMin", "craft"}},
            {{"Craft", "boundingBoxMax"}, {"Craft.BoundMax", "craft"}},
            {{"Craft", "meanAnomaly"}, {"Craft.MeanAnomaly", "craft"}},
            {{"Craft", "meanMotion"}, {"Craft.MeanMotion", "craft"}},
            {{"Craft", "periapsisArgument"}, {"Craft.PeriapsisArgument", "craft"}},
            {{"Craft", "rightAscension"}, {"Craft.RightAscension", "craft"}},
            {{"Craft", "trueAnomaly"}, {"Craft.TrueAnomaly", "craft"}},
            {{"Craft", "semiMajorAxis"}, {"Craft.SemiMajorAxis", "craft"}},
            {{"Craft", "semiMinorAxis"}, {"Craft.SemiMinorAxis", "craft"}},
            {{"Craft", "period"}, {"Craft.Period", "craft"}},
            {{"Craft", "apoapsisTime"}, {"Craft.ApoapsisTime", "craft"}},
            {{"Craft", "periapsisTime"}, {"Craft.PeriapsisTime", "craft"}},
            {{"Craft", "inclination"}, {"Craft.Inclination", "craft"}},
            {{"Craft", "eccentricity"}, {"Craft.Eccentricity", "craft"}},
            {{"Craft", "name"}, {"Name.Craft", "prop-name"}},
            {{"Craft", "idOf"}, {"Craft.NameToID", "craft-id"}},
            {{"Craft", "nameToID"}, {"Craft.NameToID", "craft-id"}},
            {{"Craft", "nameToID"}, {"Craft.NameToID", "craft-id"}},
            // Part expressions
            {{"Part", "activated"}, {"Part.Activated", "part"}},
            {{"Part", "dryMass"}, {"Part.DryMass", "part"}},
            {{"Part", "drag"}, {"Part.Drag", "part"}},
            {{"Part", "fuelTransfer"}, {"Part.FuelTransfer", "set-part"}},
            {{"Part", "idToName"}, {"Part.IDToName", "part"}},
            {{"Part", "mass"}, {"Part.Mass", "part"}},
            {{"Part", "name"}, {"Part.Name", "set-part"}},
            {{"Part", "partType"}, {"Part.PartType", "part"}},
            {{"Part", "position"}, {"Part.Position", "part"}},
            {{"Part", "temperature"}, {"Part.Temperature", "part"}},
            {{"Part", "thisID"}, {"Part.ThisID", "part"}},
            {{"Part", "underwater"}, {"Part.UnderWater", "part"}},
            {{"Part", "wetMass"}, {"Part.WetMass", "part"}},
            {{"Part", "idOf"}, {"Part.NameToID", "part-id"}},
            {{"Part", "localToPCI"}, {"Part.LocalToPci", "part-transform"}},
            {{"Part", "pciToLocal"}, {"Part.PciToLocal", "part-transform"}},
            {{"Part", "minPartId"}, {"Part.MinID", "part"}},
            {{"Part", "maxPartId"}, {"Part.MaxID", "part"}},
            {{"Part", "nameToID"}, {"Part.NameToID", "part-id"}},
            // Input slider 2-4
            {{"Input", "slider2"}, {"Input.Slider2", "prop-input"}},
            {{"Input", "slider3"}, {"Input.Slider3", "prop-input"}},
            {{"Input", "slider4"}, {"Input.Slider4", "prop-input"}},
            // Nav craft direction
            {{"Nav", "craftDirection"}, {"Nav.CraftDirection", "prop-nav"}},
            // Target
            {{"Target", "name"}, {"Target.Name", "prop-name"}},
            {{"Target", "planet"}, {"Target.Planet", "prop-name"}},
            // Terrain expressions
            {{"Terrain", "height"}, {"Terrain.Height", "terrain-query"}},
            {{"Terrain", "color"}, {"Terrain.Color", "terrain-query"}},
            {{"Raycast", "raycast"}, {"Raycast", "raycast-query"}},
        };
    auto it = kStdExprs.find({qualifier, identifier});
    if (it == kStdExprs.end()) {
        return nullptr;
    }
    return &it->second;
}

/**
Planet expressions (e.g. Planet.radius("Droo")) compile to Vizzy
<Planet op="..." style="..."> blocks.
*/
struct PlanetOp {
    const char *op;
    const char *style;
};

const PlanetOp *findPlanetOp(const std::string &identifier) {
    // Vizzy spells its operations in lower case, Juno keeps camel case,
    // and a few Juno names differ completely from the Vizzy op
    static const std::map<std::string, PlanetOp> kPlanetOps = {
        {"tolatlongagl", {"toLatLongAgl", "planet-to-lat-long-agl"}},
        {"tolatlongasl", {"toLatLongAsl", "planet-to-lat-long-asl"}},
        {"toposition", {"toPosition", "planet-to-position"}},
        {"topositionoversea",
         {"toPositionOverSea", "planet-to-position-asl"}},
        {"radius", {"radius", "planet"}},
        {"mass", {"mass", "planet"}},
        {"hasterrain", {"hasTerrain", "planet"}},
        {"atmospheredensity", {"atmosphereDensity", "planet"}},
        {"atmosphereheight", {"atmosphereHeight", "planet"}},
        {"atmospherescale", {"atmosphereScale", "planet"}},
        {"atmosphereelements", {"atmosphereElements", "planet"}},
        {"atmospherecomposition", {"atmosphereComposition", "planet"}},
        {"hydrosphereelement", {"hydrosphereElement", "planet"}},
        {"hydrospheredensity", {"hydrosphereDensity", "planet"}},
        {"soiradius", {"soiradius", "planet"}},
        {"solarposition", {"solarPosition", "planet"}},
        {"childplanets", {"childPlanets", "planet"}},
        {"crafts", {"crafts", "planet"}},
        {"craftids", {"craftids", "planet"}},
        {"parent", {"parent", "planet"}},
        {"structures", {"structures", "planet"}},
        {"day", {"day", "planet"}},
        {"year", {"year", "planet"}},
        {"velocity", {"velocity", "planet"}},
        {"apoapsis", {"apoapsis", "planet"}},
        {"periapsis", {"periapsis", "planet"}},
        {"period", {"period", "planet"}},
        {"apoapsistime", {"apoapsistime", "planet"}},
        {"periapsistime", {"periapsistime", "planet"}},
        {"inclination", {"inclination", "planet"}},
        {"eccentricity", {"eccentricity", "planet"}},
        {"meananomaly", {"meananomaly", "planet"}},
        {"meanmotion", {"meanmotion", "planet"}},
        {"periapsisargument", {"periapsisargument", "planet"}},
        {"rightascension", {"rightascension", "planet"}},
        {"trueanomaly", {"trueanomaly", "planet"}},
        {"semimajoraxis", {"semimajoraxis", "planet"}},
        {"semiminoraxis", {"semiminoraxis", "planet"}},
    };
    static const std::map<std::string, std::string> kAliases = {
        {"craftid", "craftids"},
        {"orbitaptime", "apoapsistime"},
        {"orbitpetime", "periapsistime"},
        {"orbitperiapsisargument", "periapsisargument"},
        {"orbitrightascension", "rightascension"},
        {"orbittrueanomaly", "trueanomaly"},
        {"orbitsemimajoraxis", "semimajoraxis"},
        {"orbitsemiminoraxis", "semiminoraxis"},
    };
    std::string key;
    key.reserve(identifier.size());
    for (const unsigned char c : identifier) {
        key.push_back(
            static_cast<char>(std::tolower(c)));
    }
    if (const auto alias = kAliases.find(key); alias != kAliases.end()) {
        key = alias->second;
    }
    auto it = kPlanetOps.find(key);
    if (it == kPlanetOps.end()) {
        return nullptr;
    }
    return &it->second;
}

/**
List expressions (e.g. ListOp.create("a,b,c")) compile to Vizzy
<ListOp op="..." style="..."> blocks.
*/
struct ListOp {
    const char *op;
    const char *style;
};

const ListOp *findListOp(const std::string &identifier) {
    static const std::map<std::string, ListOp> kListOps = {
        {"create", {"create", "list-create"}},
        {"get", {"get", "list-get"}},
        {"length", {"length", "list-length"}},
        {"index", {"index", "list-index"}},
    };
    auto it = kListOps.find(identifier);
    if (it == kListOps.end()) {
        return nullptr;
    }
    return &it->second;
}

/**
Math expressions (e.g. Math.sqrt(x)) compile to Vizzy <MathFunction>
blocks, except for the operations Vizzy keeps in a BinaryOp, a Not or an
EvaluateExpression block.
 */
struct MathBlock {
    const char *tag;
    const char *attr; // nullptr: the block carries a style only
    const char *value;
    const char *style;
};

const MathBlock *findMathBlock(const std::string &identifier) {
    static const std::map<std::string, MathBlock> kMathBlocks = {
        // Single argument functions
        {"abs", {"MathFunction", "function", "abs", "op-math"}},
        {"floor", {"MathFunction", "function", "floor", "op-math"}},
        {"ceil", {"MathFunction", "function", "ceil", "op-math"}},
        {"round", {"MathFunction", "function", "round", "op-math"}},
        {"sqrt", {"MathFunction", "function", "sqrt", "op-math"}},
        {"sin", {"MathFunction", "function", "sin", "op-math"}},
        {"cos", {"MathFunction", "function", "cos", "op-math"}},
        {"tan", {"MathFunction", "function", "tan", "op-math"}},
        {"asin", {"MathFunction", "function", "asin", "op-math"}},
        {"acos", {"MathFunction", "function", "acos", "op-math"}},
        {"atan", {"MathFunction", "function", "atan", "op-math"}},
        {"ln", {"MathFunction", "function", "ln", "op-math"}},
        {"log", {"MathFunction", "function", "log", "op-math"}},
        {"deg2rad", {"MathFunction", "function", "deg2rad", "op-math"}},
        {"rad2deg", {"MathFunction", "function", "rad2deg", "op-math"}},
        // Two argument operators Vizzy writes as BinaryOp
        {"random", {"BinaryOp", "op", "rand", "op-rand"}},
        {"min", {"BinaryOp", "op", "min", "op-min"}},
        {"max", {"BinaryOp", "op", "max", "op-max"}},
        {"atan2", {"BinaryOp", "op", "atan2", "op-atan-2"}},
        {"pow", {"BinaryOp", "op", "^", "op-exp"}},
        // Blocks of their own
        {"not", {"Not", nullptr, nullptr, "op-not"}},
        {"evaluate",
         {"EvaluateExpression", nullptr, nullptr, "evaluate-expression"}},
    };
    auto it = kMathBlocks.find(identifier);
    if (it == kMathBlocks.end()) {
        return nullptr;
    }
    return &it->second;
}

/**
String expressions (e.g. String.lengthOf(s)) compile to Vizzy <StringOp>
blocks. Juno names and argument order differ for two of them: Vizzy reads
letter(index, text) and substring(start, length, text).
 */
struct StringBlock {
    const char *op;
    const char *style;
};

const StringBlock *findStringOp(const std::string &identifier) {
    static const std::map<std::string, StringBlock> kStringOps = {
        {"join", {"join", "join"}},
        {"lengthOf", {"length", "length"}},
        {"letterOf", {"letter", "letter"}},
        {"substr", {"substring", "substring"}},
        {"strContains", {"contains", "contains"}},
        {"format", {"format", "format"}},
    };
    auto it = kStringOps.find(identifier);
    if (it == kStringOps.end()) {
        return nullptr;
    }
    return &it->second;
}

/**
Vector expressions (e.g. Vector.vecLength(v)) compile to Vizzy
<VectorOp> blocks: the component and norm reads are single input, the
operations on two vectors take two.
 */
struct VectorBlock {
    const char *op;
    const char *style;
};

const VectorBlock *findVectorOp(const std::string &identifier) {
    static const std::map<std::string, VectorBlock> kVectorOps = {
        {"vecLength", {"length", "vec-op-1"}},
        {"vecX", {"x", "vec-op-1"}},
        {"vecY", {"y", "vec-op-1"}},
        {"vecZ", {"z", "vec-op-1"}},
        {"normalize", {"norm", "vec-op-1"}},
        {"dot", {"dot", "vec-op-2"}},
        {"cross", {"cross", "vec-op-2"}},
        {"scale", {"scale", "vec-op-2"}},
        {"project", {"project", "vec-op-2"}},
        {"dist", {"dist", "vec-op-2"}},
        {"angle", {"angle", "vec-op-2"}},
        {"clamp", {"clamp", "vec-op-2"}},
    };
    auto it = kVectorOps.find(identifier);
    if (it == kVectorOps.end()) {
        return nullptr;
    }
    return &it->second;
}

/**
format/callFormat of a custom definition. Vizzy writes both as
"name |a| |b| " (instructions) or "name |a| |b|  return (0)"
(expressions), and drops the trailing gap entirely when there are no
parameters, e.g. "Throttle" / "Throttle return (0)".
 */
std::pair<std::string, std::string>
definitionFormat(const std::string &name,
                 const std::vector<std::string> &params, bool isExpression) {
    std::string paramsStr;
    std::string argsStr;
    for (std::size_t k = 0; k < params.size(); ++k) {
        if (k) {
            paramsStr += " ";
            argsStr += " ";
        }
        paramsStr += "|" + params[k] + "|";
        argsStr += "(" + std::to_string(k) + ")";
    }

    const std::string base =
        params.empty() ? name : name + " " + paramsStr + " ";
    const std::string callFormat =
        params.empty() ? name : name + " " + argsStr + " ";
    const std::string format = isExpression ? base + " return (0)" : base;
    return {format, callFormat};
}

/**
True when a literal is written as <Constant number="..."/> rather than
text. Requires the whole token to be consumed so a string such as "123abc"
stays text, and rejects inf/nan/hex which Vizzy cannot parse back.
 */
bool isNumericLiteral(const std::string &s) {
    if (s.empty())
        return false;

    std::size_t i = (s[0] == '+' || s[0] == '-') ? 1 : 0;
    if (i >= s.size())
        return false;
    bool digit = false;
    for (; i < s.size(); ++i) {
        const char c = s[i];
        if (std::isdigit(static_cast<unsigned char>(c))) {
            digit = true;
            continue;
        }
        if (c == '.' || c == 'e' || c == 'E' || c == '+' || c == '-')
            continue;
        return false;
    }
    if (!digit)
        return false;

    std::size_t used = 0;
    try {
        std::stod(s, &used);
    } catch (...) {
        return false;
    }
    return used == s.size();
}

/**
True when the enclosing block takes its boolean bare: Vizzy styles the
toggle (style="true"/"false") in value slots but leaves it plain inside
boolean operators, ternaries and the activation group setter, and the
else marker is a bare true as well.
 */
bool bareBoolParent(const char *tag) {
    const std::string_view name = tag;
    return name == "BoolOp" || name == "Not" || name == "Conditional" ||
           name == "SetActivationGroup";
}

/**
True for a string written as a vector triple such as "(1, 2, 3)". Juno has
no vector type, so a vector travels through the language as text and has to
be written back as Vizzy's vector constant.
 */
bool isVectorLiteral(const std::string &s) {
    if (s.size() < 9 || s.front() != '(' || s.back() != ')')
        return false;
    const std::string inner = s.substr(1, s.size() - 2);
    std::vector<std::string> parts;
    std::size_t start = 0;
    while (true) {
        const std::size_t comma = inner.find(',', start);
        parts.push_back(inner.substr(
            start, comma == std::string::npos ? std::string::npos
                                              : comma - start));
        if (comma == std::string::npos)
            break;
        start = comma + 1;
    }
    if (parts.size() != 3)
        return false;
    for (auto &part : parts) {
        const std::size_t first = part.find_first_not_of(" \t");
        const std::size_t last = part.find_last_not_of(" \t");
        if (first == std::string::npos)
            return false;
        if (!isNumericLiteral(part.substr(first, last - first + 1)))
            return false;
    }
    return true;
}

/** Writes a literal the way Vizzy does: numbers, booleans, then text. */
void appendConstant(pugi::xml_node parent, const std::string &value) {
    auto el = parent.append_child("Constant");
    if (value == "true" || value == "false") {
        if (!bareBoolParent(parent.name()))
            el.append_attribute("style") = value.c_str();
        el.append_attribute("bool") = value.c_str();
    } else if (isNumericLiteral(value)) {
        el.append_attribute("number") = value.c_str();
    } else {
        el.append_attribute("text") = value.c_str();
    }
}

/**
Writes a quoted string. The quotes are gone by the time the compiler sees
the value, so "10000.99" has to stay text here even though the same
characters as a bare token are a number; a vector triple becomes Vizzy's
vector constant.
 */
void appendStringConstant(pugi::xml_node parent, const std::string &value) {
    auto el = parent.append_child("Constant");
    if (isVectorLiteral(value)) {
        el.append_attribute("vector") = value.c_str();
    } else {
        el.append_attribute("text") = value.c_str();
    }
}

/**
Text of the negated numeric literal, or "" when the value is not a number.
Used where Vizzy expects the amount itself, e.g. "x -= 2" becomes
ChangeVariable(x, -2).
 */
std::string negateNumericLiteral(const std::string &s) {
    if (!isNumericLiteral(s))
        return "";
    double d = -std::stod(s);
    if (d == 0.0)
        return "0";
    std::string out = std::to_string(d);
    if (out.find('.') != std::string::npos) {
        out.erase(out.find_last_not_of('0') + 1, std::string::npos);
        if (out.back() == '.')
            out.pop_back();
    }
    return out;
}

std::pair<std::string, std::string> eventName(const std::string &junoEvent) {
    if (junoEvent == "start")
        return {"FlightStart", "flight-start"};
    if (junoEvent == "update")
        return {"ReceiveMessage", "receive-msg"};
    if (junoEvent == "collision")
        return {"PartCollision", "part-collision"};
    if (junoEvent == "dock")
        return {"Docked", "craft-docked"};
    if (junoEvent == "explode")
        return {"PartExplode", "part-explode"};
    if (junoEvent == "enterSOI")
        return {"ChangeSoi", "change-soi"};
    // Any other event is a custom Vizzy ReceiveMessage broadcast
    return {"ReceiveMessage", "receive-msg"};
}

/**
True when a top level declaration carries a value that Vizzy would not
know about: globals always start at 0 and lists start empty, so the source
values have to be assigned from inside a handler.
 */
bool hasTopLevelInits(const RootNode &root) {
    for (const auto &node : root.body) {
        if (const auto *varDecl = std::get_if<VarDeclaration>(&node)) {
            if (varDecl->value)
                return true;
        } else if (const auto *listDecl =
                       std::get_if<ListDeclaration>(&node)) {
            if (!listDecl->values.empty())
                return true;
        }
    }
    return false;
}

} // namespace

std::string Compiler::nextThreadPos() {
    // One column per thread, every thread starting on the same canvas row,
    // matching how Vizzy lays out independent event handlers.
    const std::string pos = std::format("{},-20", mThreadX);
    mThreadX += 400;
    return pos;
}

std::string Compiler::compileAST(const RootNode &root) {
    mThreadX = -10;
    mHandlerParams.clear();

    pugi::xml_document doc;
    auto prog = doc.append_child("Program");
    prog.append_attribute("name") = root.programName.c_str();

    // ---- Variables ----
    auto varsEl = prog.append_child("Variables");
    std::vector<std::string> order = mVariableOrder;
    std::set<std::string> emitted(order.begin(), order.end());
    for (const auto &[key, name] : mVariables) {
        if (emitted.insert(name).second)
            order.push_back(name);
    }
    for (const auto &name : order) {
        auto v = varsEl.append_child("Variable");
        v.append_attribute("name") = name.c_str();
        if (mLists.count(name)) {
            // A Vizzy list is a variable carrying an empty <Items/> container
            v.append_child("Items");
        } else {
            v.append_attribute("number") = "0";
        }
    }

    std::size_t id = 0;
    auto nextId = [&id] { return std::to_string(id++); };

    // ---- Instruction threads ----
    // Vizzy keeps every event handler and every custom instruction
    // definition in its own <Instructions> thread instead of piling them
    // into one container.
    bool hasThread = false;
    bool hasStart = false;
    for (const auto &node : root.body) {
        std::visit(
            Overload{
                [&](const EventNode &event) {
                    hasThread = true;
                    const bool isStart = event.identifier == "start";
                    hasStart = hasStart || isStart;
                    auto threadEl = prog.append_child("Instructions");
                    auto [xmlEvent, xmlStyle] = eventName(event.identifier);
                    auto el = threadEl.append_child("Event");
                    el.append_attribute("event") = xmlEvent.c_str();
                    el.append_attribute("id") = nextId().c_str();
                    el.append_attribute("style") = xmlStyle.c_str();
                    el.append_attribute("pos") = nextThreadPos().c_str();

                    // Vizzy emits a text constant child for receive-msg
                    // events (e.g. on update() => ReceiveMessage with
                    // Constant text="update").
                    if (xmlStyle == "receive-msg") {
                        auto c = el.append_child("Constant");
                        c.append_attribute("canReplace") = "false";
                        c.append_attribute("text") =
                            event.identifier.c_str();
                    }

                    mScopePath.push_back(event.identifier);

                    // Top level declarations keep the values they were
                    // compiled with by assigning them at the very top of
                    // start, before anything else the program does
                    if (isStart)
                        emitTopLevelInits(root, threadEl, nextId);

                    // A custom event carries its parameters in Vizzy's
                    // built-in data list, indexed by position; the internal
                    // events bind names Vizzy provides itself
                    const bool isMessage = xmlStyle == "receive-msg";
                    for (std::size_t k = 0; k < event.params.size(); ++k)
                        mHandlerParams[event.params[k]] =
                            isMessage ? static_cast<int>(k) + 1 : -1;

                    emitBody(event.body, threadEl, nextId);
                    mHandlerParams.clear();
                    mScopePath.pop_back();
                },
                [&](const FunctionNode &func) {
                    hasThread = true;
                    auto threadEl = prog.append_child("Instructions");
                    auto el = threadEl.append_child("CustomInstruction");
                    auto [format, callFormat] =
                        definitionFormat(func.identifier, func.params, false);
                    el.append_attribute("callFormat") = callFormat.c_str();
                    el.append_attribute("format") = format.c_str();
                    el.append_attribute("name") = func.identifier.c_str();
                    el.append_attribute("id") = nextId().c_str();
                    el.append_attribute("style") = "custom-instruction";
                    el.append_attribute("pos") = nextThreadPos().c_str();
                    mScopePath.push_back(func.identifier);
                    // Instruction parameters are local references bound by
                    // Vizzy itself, referenced as local="true"
                    for (const auto &p : func.params)
                        mHandlerParams[p] = -1;
                    emitBody(func.body, threadEl, nextId);
                    mHandlerParams.clear();
                    mScopePath.pop_back();
                },
                [](const auto &) {
                    // imports, top level vars, handled elsewhere
                },
            },
            node);
    }

    if (!hasStart && hasTopLevelInits(root)) {
        // Values to assign but no start handler to assign them from: the
        // declarations get a FlightStart thread of their own
        hasThread = true;
        auto threadEl = prog.append_child("Instructions");
        auto el = threadEl.append_child("Event");
        el.append_attribute("event") = "FlightStart";
        el.append_attribute("id") = nextId().c_str();
        el.append_attribute("style") = "flight-start";
        el.append_attribute("pos") = nextThreadPos().c_str();
        emitTopLevelInits(root, threadEl, nextId);
    } else if (!hasThread) {
        // A program without any handler still needs the container Vizzy reads
        prog.append_child("Instructions");
    }

    // ---- Expressions ----
    auto exprsEl = prog.append_child("Expressions");
    for (const auto &node : root.body) {
        std::visit(
            Overload{
                [&](const CustomExpression &customExpr) {
                    auto el = exprsEl.append_child("CustomExpression");
                    auto [format, callFormat] =
                        definitionFormat(customExpr.identifier,
                                         customExpr.params, true);
                    el.append_attribute("callFormat") = callFormat.c_str();
                    el.append_attribute("format") = format.c_str();
                    el.append_attribute("name") =
                        customExpr.identifier.c_str();
                    el.append_attribute("style") = "custom-expression";
                    el.append_attribute("pos") = nextThreadPos().c_str();

                    // Expression parameters are local references bound by
                    // Vizzy itself, referenced as local="true"
                    for (const auto &p : customExpr.params)
                        mHandlerParams[p] = -1;

                    if (customExpr.expression)
                        emitExpression(*customExpr.expression, el);
                    mHandlerParams.clear();
                },
                [](const auto &) {},
            },
            node);
    }

    std::ostringstream oss;
    pugi::xml_writer_stream writer(oss);
    doc.save(writer, PUGIXML_TEXT("  "), pugi::format_default,
             pugi::encoding_utf8);
    std::string xml = oss.str();
    // pugixml writes <?xml version="1.0"?>, add encoding to match Vizzy format
    size_t declPos = xml.find("<?xml");
    if (declPos != std::string::npos) {
        size_t endPos = xml.find("?>", declPos);
        if (endPos != std::string::npos) {
            xml.insert(endPos, " encoding=\"utf-8\"");
        }
    }
    return xml;
}

std::expected<void, FileHandlerError> Compiler::writeToFile(
    const std::filesystem::path &path, const std::string &xml) {
    return FileHandler::writeToFile(path, xml);
}

void Compiler::emitBody(const Body &body, pugi::xml_node parent,
                        std::function<std::string()> nextId) {
    for (std::size_t index = 0; index < body.body.size(); ++index) {
        if (body.body[index]) {
            emitStatement(*body.body[index], index, parent, nextId);
        }
    }
}

bool Compiler::emitTopLevelInits(const RootNode &root,
                                 pugi::xml_node parent,
                                 std::function<std::string()> nextId) {
    bool emitted = false;
    for (const auto &node : root.body) {
        std::visit(
            Overload{
                [&](const VarDeclaration &varDecl) {
                    // Without a value the declaration already matches the
                    // 0 every Vizzy global starts at
                    if (!varDecl.value)
                        return;
                    emitVarDeclaration(varDecl, parent, nextId);
                    emitted = true;
                },
                [&](const ListDeclaration &listDecl) {
                    // An empty list already matches the empty <Items/>
                    if (listDecl.values.empty())
                        return;
                    emitListDeclaration(listDecl, parent, nextId);
                    emitted = true;
                },
                [](const auto &) {},
            },
            node);
    }
    return emitted;
}

void Compiler::emitVarDeclaration(const VarDeclaration &varDecl,
                                  pugi::xml_node parent,
                                  std::function<std::string()> nextId) {
    auto el = parent.append_child("SetVariable");
    el.append_attribute("id") = nextId().c_str();
    el.append_attribute("style") = "set-variable";
    auto ref = el.append_child("Variable");
    ref.append_attribute("list") = "false";
    ref.append_attribute("local") = "false";
    ref.append_attribute("variableName") =
        resolveVariableName(varDecl.identifier).c_str();
    if (varDecl.value) {
        emitExpression(*varDecl.value, el);
    } else {
        auto c = el.append_child("Constant");
        c.append_attribute("number") = "0";
    }
}

void Compiler::emitListDeclaration(const ListDeclaration &listDecl,
                                   pugi::xml_node parent,
                                   std::function<std::string()> nextId) {
    auto el = parent.append_child("SetVariable");
    el.append_attribute("id") = nextId().c_str();
    el.append_attribute("style") = "list-init";
    auto ref = el.append_child("Variable");
    ref.append_attribute("list") = "true";
    ref.append_attribute("local") = "false";
    ref.append_attribute("variableName") =
        resolveVariableName(listDecl.identifier).c_str();
    auto listOp = el.append_child("ListOp");
    listOp.append_attribute("op") = "create";
    listOp.append_attribute("style") = "list-create";
    std::string csv;
    for (std::size_t k = 0; k < listDecl.values.size(); ++k) {
        if (k)
            csv += ",";
        csv += std::visit(
            Overload{
                [](const Value &v) { return v.value; },
                [](const auto &) { return std::string(""); },
            },
            listDecl.values[k]->value);
    }
    auto c = listOp.append_child("Constant");
    c.append_attribute("text") = csv.c_str();
}

void Compiler::emitStatement(const Statements &statement, std::size_t index,
                             pugi::xml_node parent,
                             std::function<std::string()> nextId) {
    std::visit(
        Overload{
            [&](const Body &body) {
                mScopePath.push_back(std::to_string(index));
                auto inner = parent.append_child("Instructions");
                emitBody(body, inner, nextId);
                mScopePath.pop_back();
            },
            [&](const VarDeclaration &varDecl) {
                emitVarDeclaration(varDecl, parent, nextId);
            },
            [&](const VarAssignment &assign) {
                // Vizzy splits writing a variable in two: SetVariable
                // stores a value, ChangeVariable adds an amount, and a
                // list element is reached through SetList. The compound
                // operators therefore have to survive instead of
                // collapsing into a plain assignment.
                auto appendVarRef = [&](pugi::xml_node into,
                                        const char *isList) {
                    auto ref = into.append_child("Variable");
                    ref.append_attribute("list") = isList;
                    ref.append_attribute("local") = "false";
                    ref.append_attribute("variableName") =
                        resolveVariableName(assign.identifier).c_str();
                };

                // The element a compound list write starts from
                auto appendListGet = [&](pugi::xml_node into) {
                    auto get = into.append_child("ListOp");
                    get.append_attribute("op") = "get";
                    get.append_attribute("style") = "list-get";
                    appendVarRef(get, "true");
                    if (assign.indexExpr)
                        emitListIndex(*assign.indexExpr, get);
                };

                auto appendValue = [&](pugi::xml_node into) {
                    if (assign.value) {
                        emitExpression(*assign.value, into);
                    } else {
                        auto c = into.append_child("Constant");
                        c.append_attribute("number") = "0";
                    }
                };

                // x -= amount is "change by -amount"; Vizzy has no
                // subtracting variant of the block
                auto appendNegatedValue = [&](pugi::xml_node into) {
                    if (assign.value) {
                        if (const auto *v =
                                std::get_if<Value>(&assign.value->value);
                            v && v->kind == Value::Kind::Literal) {
                            if (auto neg =
                                    negateNumericLiteral(v->value);
                                !neg.empty()) {
                                auto c = into.append_child("Constant");
                                c.append_attribute("number") =
                                    neg.c_str();
                                return;
                            }
                        }
                    }
                    auto bin = into.append_child("BinaryOp");
                    bin.append_attribute("op") = "-";
                    bin.append_attribute("style") = "op-sub";
                    auto zero = bin.append_child("Constant");
                    zero.append_attribute("number") = "0";
                    appendValue(bin);
                };

                if (assign.indexExpr) {
                    // nums[i] = v -> SetList with the list, the index and
                    // the new element value in that order
                    auto el = parent.append_child("SetList");
                    el.append_attribute("op") = "set";
                    el.append_attribute("id") = nextId().c_str();
                    el.append_attribute("style") = "list-set";
                    appendVarRef(el, "true");
                    emitListIndex(*assign.indexExpr, el);
                    if (assign.op == VarAssignment::ASSIGN) {
                        appendValue(el);
                    } else {
                        static const char *ops[] = {"+", "-", "*", "/"};
                        static const char *styles[] = {
                            "op-add", "op-sub", "op-mul", "op-div"};
                        const std::size_t k =
                            assign.op == VarAssignment::PLUS_ASSIGN
                                ? 0
                                : assign.op == VarAssignment::MINUS_ASSIGN
                                      ? 1
                                      : assign.op ==
                                                VarAssignment::MULT_ASSIGN
                                            ? 2
                                            : 3;
                        auto bin = el.append_child("BinaryOp");
                        bin.append_attribute("op") = ops[k];
                        bin.append_attribute("style") = styles[k];
                        appendListGet(bin);
                        appendValue(bin);
                    }
                    return;
                }

                if (assign.op == VarAssignment::PLUS_ASSIGN ||
                    assign.op == VarAssignment::MINUS_ASSIGN) {
                    auto el = parent.append_child("ChangeVariable");
                    el.append_attribute("id") = nextId().c_str();
                    el.append_attribute("style") = "change-variable";
                    appendVarRef(el, "false");
                    if (assign.op == VarAssignment::PLUS_ASSIGN)
                        appendValue(el);
                    else
                        appendNegatedValue(el);
                    return;
                }

                auto el = parent.append_child("SetVariable");
                el.append_attribute("id") = nextId().c_str();
                el.append_attribute("style") = "set-variable";
                appendVarRef(el, "false");
                if (assign.op == VarAssignment::ASSIGN) {
                    appendValue(el);
                } else {
                    // *= and /= have no dedicated block: store the
                    // variable combined with the right hand side
                    const bool divide =
                        assign.op == VarAssignment::DIVIDE_ASSIGN;
                    auto bin = el.append_child("BinaryOp");
                    bin.append_attribute("op") = divide ? "/" : "*";
                    bin.append_attribute("style") =
                        divide ? "op-div" : "op-mul";
                    appendVarRef(bin, "false");
                    appendValue(bin);
                }
            },
            [&](const ListDeclaration &listDecl) {
                emitListDeclaration(listDecl, parent, nextId);
            },
            [&](const DoInstruction &instr) {
                if (instr.qualifier == "Game") {
                    if (instr.identifier == "display") {
                        auto el = parent.append_child("DisplayMessage");
                        el.append_attribute("id") = nextId().c_str();
                        el.append_attribute("style") = "display";
                        // Vizzy always carries text plus a duration
                        if (instr.params.empty()) {
                            auto c = el.append_child("Constant");
                            c.append_attribute("text") = "";
                        }
                        for (const auto &p : instr.params) {
                            emitExpression(*p, el);
                        }
                        if (instr.params.size() < 2) {
                            auto c = el.append_child("Constant");
                            c.append_attribute("number") = "7";
                        }
                    } else if (instr.identifier == "delay") {
                        auto el = parent.append_child("WaitSeconds");
                        el.append_attribute("id") = nextId().c_str();
                        el.append_attribute("style") = "wait-seconds";
                        for (const auto &p : instr.params) {
                            emitExpression(*p, el);
                        }
                    } else if (instr.identifier == "delayUntil") {
                        auto el = parent.append_child("WaitUntil");
                        el.append_attribute("id") = nextId().c_str();
                        el.append_attribute("style") = "wait-until";
                        for (const auto &p : instr.params) {
                            emitExpression(*p, el);
                        }
                    } else if (instr.identifier == "switchToCraft") {
                        auto el = parent.append_child("SwitchCraft");
                        el.append_attribute("id") = nextId().c_str();
                        el.append_attribute("style") = "switch-craft";
                        for (const auto &p : instr.params) {
                            emitExpression(*p, el);
                        }
                    } else if (instr.identifier == "flightLog") {
                        auto el = parent.append_child("LogFlight");
                        el.append_attribute("id") = nextId().c_str();
                        el.append_attribute("style") = "flightlog";
                        for (const auto &p : instr.params) {
                            emitExpression(*p, el);
                        }
                        if (instr.params.size() == 1) {
                            auto def = el.append_child("Constant");
                            def.append_attribute("style") = "true";
                            def.append_attribute("bool") = "true";
                        }
                    } else if (instr.identifier == "beep"
                               || instr.identifier == "sineBeep"
                               || instr.identifier == "sineTriangle"
                               || instr.identifier == "sineSaw"
                               || instr.identifier == "sineSquare"
                               || instr.identifier == "sineNoise") {
                        auto el = parent.append_child("PlayBeep");
                        // Vizzy writes the waveform ahead of the id
                        const char *waveform = "Sine";
                        if (instr.identifier == "sineTriangle") {
                            waveform = "Triangle";
                        } else if (instr.identifier == "sineSaw") {
                            waveform = "Saw";
                        } else if (instr.identifier == "sineSquare") {
                            waveform = "Square";
                        } else if (instr.identifier == "sineNoise") {
                            waveform = "Noise";
                        }
                        el.append_attribute("waveform") = waveform;
                        el.append_attribute("id") = nextId().c_str();
                        el.append_attribute("style") = "play-beep-waveform";
                        for (const auto &p : instr.params) {
                            emitExpression(*p, el);
                        }
                    } else if (instr.identifier == "speak") {
                        auto el = parent.append_child("Speak");
                        el.append_attribute("id") = nextId().c_str();
                        el.append_attribute("style") = "speak";
                        for (const auto &p : instr.params) {
                            emitExpression(*p, el);
                        }
                    } else {
                        auto el = parent.append_child("SetCraftProperty");
                        el.append_attribute("property") =
                            instr.identifier.c_str();
                        el.append_attribute("id") = nextId().c_str();
                        el.append_attribute("style") = "set-part";
                        for (const auto &p : instr.params) {
                            emitExpression(*p, el);
                        }
                    }
                } else if (instr.qualifier == "Input") {
                    // Vizzy uses SetInput for input controls, not
                    // SetCraftProperty. The activation group is a block of
                    // its own, and a slider's number names the input
                    // instead of travelling as a value.
                    const auto *sliderIndex =
                        instr.identifier == "slider" &&
                                instr.params.size() == 2
                            ? std::get_if<Value>(&instr.params[0]->value)
                            : nullptr;
                    const bool fixedSlider =
                        sliderIndex &&
                        sliderIndex->kind == Value::Kind::Literal &&
                        sliderIndex->value.size() == 1 &&
                        sliderIndex->value[0] >= '1' &&
                        sliderIndex->value[0] <= '4';
                    if (instr.identifier == "activationGroup") {
                        auto el =
                            parent.append_child("SetActivationGroup");
                        el.append_attribute("id") = nextId().c_str();
                        el.append_attribute("style") = "set-ag";
                        for (const auto &p : instr.params) {
                            emitExpression(*p, el);
                        }
                    } else if (fixedSlider) {
                        auto el = parent.append_child("SetInput");
                        el.append_attribute("input") =
                            ("slider" +
                             std::string(1, sliderIndex->value[0]))
                                .c_str();
                        el.append_attribute("id") = nextId().c_str();
                        el.append_attribute("style") = "set-input";
                        emitExpression(*instr.params[1], el);
                    } else {
                        auto el = parent.append_child("SetInput");
                        el.append_attribute("input") =
                            instr.identifier.c_str();
                        el.append_attribute("id") = nextId().c_str();
                        el.append_attribute("style") = "set-input";
                        for (const auto &p : instr.params) {
                            emitExpression(*p, el);
                        }
                    }
                } else if (instr.qualifier == "Camera") {
                    // Vizzy keeps the camera controls in SetCameraProperty
                    // and spells the rotations rotationX/rotationY
                    static const std::map<std::string, std::string>
                        kCameraProperties = {
                            {"zoom", "zoom"},
                            {"xRotation", "rotationX"},
                            {"yRotation", "rotationY"},
                            {"tilt", "tilt"},
                            {"mode", "mode"},
                            {"modeIndex", "modeIndex"},
                            {"targetOffset", "targetOffset"},
                        };
                    const auto property =
                        kCameraProperties.find(instr.identifier);
                    auto el = parent.append_child("SetCameraProperty");
                    el.append_attribute("property") =
                        property != kCameraProperties.end()
                            ? property->second.c_str()
                            : instr.identifier.c_str();
                    el.append_attribute("id") = nextId().c_str();
                    el.append_attribute("style") = "set-camera";
                    for (const auto &p : instr.params) {
                        emitExpression(*p, el);
                    }
                } else if (instr.qualifier == "Lists") {
                    // Vizzy's list setters carry the list first, then the
                    // value, then the index
                    static const std::map<std::string,
                                          std::pair<const char *, const char *>>
                        kListSets = {
                            {"add", {"add", "list-add"}},
                            {"insert", {"insert", "list-insert"}},
                            {"remove", {"remove", "list-remove"}},
                            {"set", {"set", "list-set"}},
                            {"clear", {"clear", "list-clear"}},
                            {"sort", {"sort", "list-sort"}},
                            {"reverse", {"reverse", "list-reverse"}},
                        };
                    auto entry = kListSets.find(instr.identifier);
                    if (entry == kListSets.end()) {
                        // Not a setter Vizzy knows: keep the old shape
                        auto el = parent.append_child("SetCraftProperty");
                        el.append_attribute("property") =
                            instr.identifier.c_str();
                        el.append_attribute("id") = nextId().c_str();
                        el.append_attribute("style") = "set-part";
                        for (const auto &p : instr.params) {
                            emitExpression(*p, el);
                        }
                        return;
                    }
                    const char *op = entry->second.first;
                    const char *style = entry->second.second;
                    // Inserting a single value without an index appends,
                    // which is Vizzy's add block
                    if (instr.identifier == "insert" &&
                        instr.params.size() == 2) {
                        op = "add";
                        style = "list-add";
                    }
                    auto el = parent.append_child("SetList");
                    el.append_attribute("op") = op;
                    el.append_attribute("id") = nextId().c_str();
                    el.append_attribute("style") = style;
                    if (instr.identifier == "set" &&
                        instr.params.size() == 3) {
                        // Juno writes (list, index, value), Vizzy reads
                        // (list, value, index)
                        emitExpression(*instr.params[0], el);
                        emitExpression(*instr.params[2], el);
                        emitExpression(*instr.params[1], el);
                    } else {
                        for (const auto &p : instr.params) {
                            emitExpression(*p, el);
                        }
                    }
                } else if (instr.qualifier == "Part") {
                    // Vizzy prefixes the part setters with Part.
                    static const std::map<std::string, std::string>
                        kPartProperties = {
                            {"activate", "Part.SetActivated"},
                            {"focus", "Part.SetFocused"},
                            {"name", "Part.SetName"},
                            {"explode", "Part.SetExplode"},
                            {"drainFuel", "Part.FuelTransfer"},
                        };
                    const auto property =
                        kPartProperties.find(instr.identifier);
                    auto el = parent.append_child("SetCraftProperty");
                    el.append_attribute("property") =
                        property != kPartProperties.end()
                            ? property->second.c_str()
                            : instr.identifier.c_str();
                    el.append_attribute("id") = nextId().c_str();
                    el.append_attribute("style") = "set-part";
                    for (const auto &p : instr.params) {
                        emitExpression(*p, el);
                    }
                } else if (instr.qualifier.empty()) {
                    // An unqualified do is a call to a user defined
                    // `instr`, Vizzy's CallCustomInstruction block
                    auto el = parent.append_child("CallCustomInstruction");
                    el.append_attribute("call") = instr.identifier.c_str();
                    el.append_attribute("id") = nextId().c_str();
                    el.append_attribute("style") = "call-custom-instruction";
                    for (const auto &p : instr.params) {
                        emitExpression(*p, el);
                    }
                } else {
                    if (instr.identifier == "activateStage") {
                        auto el = parent.append_child("ActivateStage");
                        el.append_attribute("id") = nextId().c_str();
                        el.append_attribute("style") = "activate-stage";
                    } else if (instr.identifier == "lockPrograde") {
                        auto el = parent.append_child("LockNavSphere");
                        el.append_attribute("indicatorType") = "Prograde";
                        el.append_attribute("id") = nextId().c_str();
                        el.append_attribute("style") = "lock-nav-sphere";
                    } else if (instr.identifier == "lockRetrograde") {
                        auto el = parent.append_child("LockNavSphere");
                        el.append_attribute("indicatorType") = "Retrograde";
                        el.append_attribute("id") = nextId().c_str();
                        el.append_attribute("style") = "lock-nav-sphere";
                    } else if (instr.identifier == "lockNone") {
                        auto el = parent.append_child("LockNavSphere");
                        el.append_attribute("indicatorType") = "None";
                        el.append_attribute("id") = nextId().c_str();
                        el.append_attribute("style") = "lock-nav-sphere";
                    } else if (instr.identifier == "lockTarget") {
                        auto el = parent.append_child("LockNavSphere");
                        el.append_attribute("indicatorType") = "Target";
                        el.append_attribute("id") = nextId().c_str();
                        el.append_attribute("style") = "lock-nav-sphere";
                    } else if (instr.identifier == "lockBurnNode") {
                        auto el = parent.append_child("LockNavSphere");
                        el.append_attribute("indicatorType") = "BurnNode";
                        el.append_attribute("id") = nextId().c_str();
                        el.append_attribute("style") = "lock-nav-sphere";
                    } else if (instr.identifier == "lockCurrent") {
                        auto el = parent.append_child("LockNavSphere");
                        el.append_attribute("indicatorType") = "Current";
                        el.append_attribute("id") = nextId().c_str();
                        el.append_attribute("style") = "lock-nav-sphere";
                    } else if (instr.identifier == "lockVector") {
                        auto el = parent.append_child("LockNavSphere");
                        el.append_attribute("indicatorType") = "Vector";
                        el.append_attribute("id") = nextId().c_str();
                        el.append_attribute("style") =
                            "lock-nav-sphere-vector";
                        // Three plain numbers make up the one vector
                        // constant Vizzy plugs into the block
                        std::string triple;
                        bool foldable = instr.params.size() == 3;
                        for (const auto &p : instr.params) {
                            const auto *v =
                                std::get_if<Value>(&p->value);
                            if (!v || v->kind != Value::Kind::Literal ||
                                !isNumericLiteral(v->value)) {
                                foldable = false;
                                break;
                            }
                            if (!triple.empty())
                                triple += ", ";
                            triple += v->value;
                        }
                        if (foldable) {
                            auto c = el.append_child("Constant");
                            c.append_attribute("vector") =
                                ("(" + triple + ")").c_str();
                        } else {
                            for (const auto &p : instr.params) {
                                emitExpression(*p, el);
                            }
                        }
                    } else if (instr.identifier == "setTarget") {
                        auto el = parent.append_child("SetTarget");
                        el.append_attribute("id") = nextId().c_str();
                        el.append_attribute("style") = "set-target";
                        for (const auto &p : instr.params) {
                            emitExpression(*p, el);
                        }
                    } else if (instr.identifier == "pidPitchTo") {
                        auto el = parent.append_child("SetTargetHeading");
                        el.append_attribute("property") = "pid-pitch";
                        el.append_attribute("id") = nextId().c_str();
                        el.append_attribute("style") = "set-heading";
                        for (const auto &p : instr.params) {
                            emitExpression(*p, el);
                        }
                    } else if (instr.identifier == "pidRollTo") {
                        auto el = parent.append_child("SetTargetHeading");
                        el.append_attribute("property") = "pid-roll";
                        el.append_attribute("id") = nextId().c_str();
                        el.append_attribute("style") = "set-heading";
                        for (const auto &p : instr.params) {
                            emitExpression(*p, el);
                        }
                    } else if (instr.identifier == "pitchTo") {
                        auto el = parent.append_child("SetTargetHeading");
                        el.append_attribute("property") = "pitch";
                        el.append_attribute("id") = nextId().c_str();
                        el.append_attribute("style") = "set-heading";
                        for (const auto &p : instr.params) {
                            emitExpression(*p, el);
                        }
                    } else if (instr.identifier == "headingTo") {
                        auto el = parent.append_child("SetTargetHeading");
                        el.append_attribute("property") = "heading";
                        el.append_attribute("id") = nextId().c_str();
                        el.append_attribute("style") = "set-heading";
                        for (const auto &p : instr.params) {
                            emitExpression(*p, el);
                        }
                    } else {
                        auto el = parent.append_child("SetCraftProperty");
                        el.append_attribute("property") =
                            instr.identifier.c_str();
                        el.append_attribute("id") = nextId().c_str();
                        el.append_attribute("style") = "set-part";
                        for (const auto &p : instr.params) {
                            emitExpression(*p, el);
                        }
                    }
                }
            },
            [&](const BreakStatement &) {
                auto el = parent.append_child("Break");
                el.append_attribute("id") = nextId().c_str();
                el.append_attribute("style") = "break";
            },
            [&](const ConditionalStatement &cond) {
                auto ifEl = parent.append_child("If");
                ifEl.append_attribute("id") = nextId().c_str();
                ifEl.append_attribute("style") = "if";
                emitCondition(*cond.ifStatement.condition, ifEl);
                mScopePath.push_back(std::to_string(index) + ".if");
                auto ifBody = ifEl.append_child("Instructions");
                emitBody(cond.ifStatement.body, ifBody, nextId);
                mScopePath.pop_back();

                for (std::size_t i = 0; i < cond.elifStatements.size(); ++i) {
                    auto ei = parent.append_child("ElseIf");
                    ei.append_attribute("id") = nextId().c_str();
                    ei.append_attribute("style") = "else-if";
                    emitCondition(*cond.elifStatements[i].condition, ei);
                    mScopePath.push_back(
                        std::to_string(index) + ".elif" + std::to_string(i));
                    auto eiBody = ei.append_child("Instructions");
                    emitBody(cond.elifStatements[i].body, eiBody, nextId);
                    mScopePath.pop_back();
                }

                // Vizzy only writes an else block when the source has one,
                // and marks it with a bare true constant
                if (!cond.elseStatement.body.empty()) {
                    auto elseEl = parent.append_child("ElseIf");
                    elseEl.append_attribute("id") = nextId().c_str();
                    elseEl.append_attribute("style") = "else";
                    auto c = elseEl.append_child("Constant");
                    c.append_attribute("bool") = "true";
                    mScopePath.push_back(std::to_string(index) + ".else");
                    auto elseBody = elseEl.append_child("Instructions");
                    emitBody(cond.elseStatement, elseBody, nextId);
                    mScopePath.pop_back();
                }
            },
            [&](const EmitEvent &emit) {
                auto el = parent.append_child("BroadcastMessage");
                // The reference programs only ever pair these three styles
                // with these two flags: broadcast-msg (global=false
                // local=true), broadcast-msg-craft (false/false) and
                // broadcast-msg-all-crafts (true/true) for other crafts.
                const char *style = "broadcast-msg";
                bool global = false;
                bool local = true;
                if (emit.option == EmitEvent::TO_CRAFT) {
                    style = "broadcast-msg-craft";
                    local = false;
                } else if (emit.option == EmitEvent::TO_NEARBY_CRAFTS) {
                    style = "broadcast-msg-all-crafts";
                    global = true;
                }
                el.append_attribute("global") = global ? "true" : "false";
                el.append_attribute("local") = local ? "true" : "false";
                el.append_attribute("id") = nextId().c_str();
                el.append_attribute("style") = style;
                auto mc = el.append_child("Constant");
                mc.append_attribute("text") = emit.identifier.c_str();
                // The block always carries a data input, and a custom event
                // reads it as Vizzy's `data` list, so the parameters travel
                // as one list: nothing to index with no parameters, the
                // single value on its own, one CSV constant when they are
                // all literals, and a join with "," separators otherwise
                if (emit.params.empty()) {
                    auto dc = el.append_child("Constant");
                    dc.append_attribute("number") = "0";
                } else if (emit.params.size() == 1) {
                    auto data = el.append_child("ListOp");
                    data.append_attribute("op") = "create";
                    data.append_attribute("style") = "list-create";
                    emitExpression(*emit.params[0], data);
                } else {
                    auto data = el.append_child("ListOp");
                    data.append_attribute("op") = "create";
                    data.append_attribute("style") = "list-create";
                    bool allLiterals = true;
                    for (const auto &p : emit.params) {
                        const auto *v = std::get_if<Value>(&p->value);
                        if (!v || (v->kind != Value::Kind::Literal &&
                                   v->kind != Value::Kind::String))
                            allLiterals = false;
                    }
                    if (allLiterals) {
                        std::string csv;
                        for (const auto &p : emit.params) {
                            if (!csv.empty())
                                csv += ",";
                            csv += std::get<Value>(p->value).value;
                        }
                        auto c = data.append_child("Constant");
                        c.append_attribute("text") = csv.c_str();
                    } else {
                        auto join = data.append_child("StringOp");
                        join.append_attribute("op") = "join";
                        join.append_attribute("style") = "join";
                        for (std::size_t k = 0; k < emit.params.size(); ++k) {
                            if (k) {
                                auto sep =
                                    join.append_child("Constant");
                                sep.append_attribute("text") = ",";
                            }
                            emitExpression(*emit.params[k], join);
                        }
                    }
                }
            },
            [&](const RepeatLoop &repeat) {
                auto el = parent.append_child("Repeat");
                el.append_attribute("id") = nextId().c_str();
                el.append_attribute("style") = "repeat";
                if (repeat.repeatNumber) {
                    emitExpression(*repeat.repeatNumber, el);
                } else {
                    auto c = el.append_child("Constant");
                    c.append_attribute("number") = "1";
                }
                mScopePath.push_back(std::to_string(index));
                auto bodyEl = el.append_child("Instructions");
                emitBody(repeat.body, bodyEl, nextId);
                mScopePath.pop_back();
            },
            [&](const WhileLoop &loop) {
                auto el = parent.append_child("While");
                el.append_attribute("id") = nextId().c_str();
                el.append_attribute("style") = "while";
                emitCondition(*loop.condition, el);
                mScopePath.push_back(std::to_string(index));
                auto bodyEl = el.append_child("Instructions");
                emitBody(loop.body, bodyEl, nextId);
                mScopePath.pop_back();
            },
            [&](const ForLoop &loop) {
                // Push for-loop scope so iterator resolves to its hashed name
                mScopePath.push_back(std::to_string(index));

                // 1. Init: i = start (sibling at parent level)
                auto initEl = parent.append_child("SetVariable");
                initEl.append_attribute("id") = nextId().c_str();
                initEl.append_attribute("style") = "set-variable";
                {
                    auto ref = initEl.append_child("Variable");
                    ref.append_attribute("list") = "false";
                    ref.append_attribute("local") = "false";
                    ref.append_attribute("variableName") =
                        resolveVariableName(loop.iterator).c_str();
                }
                if (loop.start)
                    emitExpression(*loop.start, initEl);
                else {
                    auto c = initEl.append_child("Constant");
                    c.append_attribute("number") = "0";
                }

                // 2. While loop (sibling at parent level)
                auto whileEl = parent.append_child("While");
                whileEl.append_attribute("id") = nextId().c_str();
                whileEl.append_attribute("style") = "while";

                // Determine loop direction from `by`: negative step means
                // counting down, so condition should be i >= end instead
                // of i <= end.
                bool countingDown = false;
                if (loop.by) {
                    if (std::holds_alternative<Value>(loop.by->value)) {
                        const Value &byVal = std::get<Value>(loop.by->value);
                        if (byVal.kind == Value::Kind::Literal) {
                            try {
                                double byNum = std::stod(byVal.value);
                                countingDown = byNum < 0;
                            } catch (...) {
                                countingDown = false;
                            }
                        }
                    } else if (std::holds_alternative<UnaryOp>(
                                   loop.by->value)) {
                        const UnaryOp &un = std::get<UnaryOp>(loop.by->value);
                        if (un.operation == UnaryOp::Operation::NEGATE &&
                            un.value &&
                            std::holds_alternative<Value>(un.value->value)) {
                            const Value &byVal =
                                std::get<Value>(un.value->value);
                            if (byVal.kind == Value::Kind::Literal) {
                                try {
                                    double byNum = std::stod(byVal.value);
                                    countingDown = byNum > 0;
                                } catch (...) {
                                    countingDown = false;
                                }
                            }
                        }
                    }
                }

                // Condition: i <= end (forward) or i >= end (backward)
                auto condEl = whileEl.append_child("Comparison");
                condEl.append_attribute("op") =
                    countingDown ? "ge" : "le";
                condEl.append_attribute("style") =
                    countingDown ? "op-gte" : "op-lte";
                {
                    auto lhsRef = condEl.append_child("Variable");
                    lhsRef.append_attribute("list") = "false";
                    lhsRef.append_attribute("local") = "false";
                    lhsRef.append_attribute("variableName") =
                        resolveVariableName(loop.iterator).c_str();
                }
                if (loop.end)
                    emitExpression(*loop.end, condEl);

                // Body: original statements + increment
                auto bodyEl = whileEl.append_child("Instructions");
                emitBody(loop.body, bodyEl, nextId);

                // Increment: i = i + by
                auto incrEl = bodyEl.append_child("SetVariable");
                incrEl.append_attribute("id") = nextId().c_str();
                incrEl.append_attribute("style") = "set-variable";
                {
                    auto incrRef = incrEl.append_child("Variable");
                    incrRef.append_attribute("list") = "false";
                    incrRef.append_attribute("local") = "false";
                    incrRef.append_attribute("variableName") =
                        resolveVariableName(loop.iterator).c_str();
                }
                {
                    auto incrOp = incrEl.append_child("BinaryOp");
                    incrOp.append_attribute("op") = "+";
                    incrOp.append_attribute("style") = "op-add";
                    {
                        auto incrLhs = incrOp.append_child("Variable");
                        incrLhs.append_attribute("list") = "false";
                        incrLhs.append_attribute("local") = "false";
                        incrLhs.append_attribute("variableName") =
                            resolveVariableName(loop.iterator).c_str();
                    }
                    if (loop.by)
                        emitExpression(*loop.by, incrOp);
                    else {
                        auto c = incrOp.append_child("Constant");
                        c.append_attribute("number") = "1";
                    }
                }

                mScopePath.pop_back();
            },
        },
        statement.statement);
}

void Compiler::emitExpression(const Expression &expr, pugi::xml_node parent) {
    std::visit(Overload{
                   [&](const Value &v) {
                       if (v.kind == Value::Kind::Literal) {
                           appendConstant(parent, v.value);
                           return;
                       }
                       if (v.kind == Value::Kind::String) {
                           appendStringConstant(parent, v.value);
                           return;
                       }

                       const std::string resolved =
                           resolveVariableName(v.value);
                       auto param = mHandlerParams.find(v.value);
                       // A parameter only wins while no local of the same
                       // name shadows it inside the handler
                       if (param != mHandlerParams.end() &&
                           resolved == v.value) {
                           if (param->second > 0) {
                               // A custom event delivers its parameters
                               // through Vizzy's built-in data list
                               auto get = parent.append_child("ListOp");
                               get.append_attribute("op") = "get";
                               get.append_attribute("style") =
                                   "list-get";
                               auto ref = get.append_child("Variable");
                               ref.append_attribute("list") = "false";
                               ref.append_attribute("local") = "true";
                               ref.append_attribute("variableName") =
                                   "data";
                               auto index =
                                   get.append_child("Constant");
                               index.append_attribute("number") =
                                   std::to_string(param->second)
                                       .c_str();
                               return;
                           }
                           // Internal events bind their own names (part,
                           // planet, ...) which Vizzy already provides
                           auto ref = parent.append_child("Variable");
                           ref.append_attribute("list") = "false";
                           ref.append_attribute("local") = "true";
                           ref.append_attribute("variableName") =
                               v.value.c_str();
                           return;
                       }

                       auto ref = parent.append_child("Variable");
                       ref.append_attribute("list") =
                           mLists.count(resolved) ? "true" : "false";
                       ref.append_attribute("local") = "false";
                       ref.append_attribute("variableName") =
                           resolved.c_str();
                   },
                   [&](const BinaryOp &bop) {
                       // Vizzy keeps comparisons and boolean operators in
                       // their own blocks whatever the context
                       if (isComparisonOp(bop.operation)) {
                           pugi::xml_node target = parent;
                           if (bop.operation ==
                               BinaryOp::Operation::NOT_EQUAL) {
                               // No not-equal operator exists in the
                               // reference programs: emit Not(=) instead
                               auto notEl = parent.append_child("Not");
                               notEl.append_attribute("style") = "op-not";
                               target = notEl;
                           }
                           auto el = target.append_child("Comparison");
                           el.append_attribute("op") =
                               comparisonOp(bop.operation);
                           // binopStyle maps != to op-eq too: the
                           // negation lives in the surrounding Not block
                           el.append_attribute("style") =
                               binopStyle(bop.operation);
                           if (bop.lhs)
                               emitExpression(*bop.lhs, el);
                           if (bop.rhs)
                               emitExpression(*bop.rhs, el);
                           return;
                       }
                       if (bop.operation == BinaryOp::Operation::AND ||
                           bop.operation == BinaryOp::Operation::OR) {
                           auto el = parent.append_child("BoolOp");
                           el.append_attribute("op") =
                               binopOp(bop.operation);
                           el.append_attribute("style") =
                               binopStyle(bop.operation);
                           if (bop.lhs)
                               emitExpression(*bop.lhs, el);
                           if (bop.rhs)
                               emitExpression(*bop.rhs, el);
                           return;
                       }
                       auto el = parent.append_child("BinaryOp");
                       el.append_attribute("op") = binopOp(bop.operation);
                       el.append_attribute("style") = binopStyle(bop.operation);
                       if (bop.lhs)
                           emitExpression(*bop.lhs, el);
                       if (bop.rhs)
                           emitExpression(*bop.rhs, el);
                   },
                    [&](const UnaryOp &uop) {
                        if (uop.operation == UnaryOp::Operation::NEGATE) {
                            // Fold a numeric literal into a negative
                            // constant, otherwise write the subtraction
                            // Vizzy expects: 0 - value
                            if (uop.value) {
                                if (const auto *v =
                                        std::get_if<Value>(
                                            &uop.value->value);
                                    v && v->kind ==
                                             Value::Kind::Literal) {
                                    if (auto neg =
                                            negateNumericLiteral(
                                                v->value);
                                        !neg.empty()) {
                                        auto el = parent.append_child(
                                            "Constant");
                                        el.append_attribute("number") =
                                            neg.c_str();
                                        return;
                                    }
                                }
                                auto el = parent.append_child(
                                    "BinaryOp");
                                el.append_attribute("op") = "-";
                                el.append_attribute("style") = "op-sub";
                                auto c = el.append_child("Constant");
                                c.append_attribute("number") = "0";
                                emitExpression(*uop.value, el);
                            }
                        } else {
                            auto el = parent.append_child("Not");
                            el.append_attribute("style") = "op-not";
                            if (uop.value)
                                emitExpression(*uop.value, el);
                        }
                    },
                   [&](const ListIndex &li) {
                       auto el = parent.append_child("ListOp");
                       el.append_attribute("op") = "get";
                       el.append_attribute("style") = "list-get";
                    auto ref = el.append_child("Variable");
                        ref.append_attribute("list") = "true";
                        ref.append_attribute("local") = "false";
                        ref.append_attribute("variableName") =
                            resolveVariableName(li.identifier).c_str();
                       if (li.index)
                           emitListIndex(*li.index, el);
                   },
                   [&](const TernaryOp &t) {
                       auto el = parent.append_child("Conditional");
                       el.append_attribute("style") = "conditional";
                       if (t.condition)
                           emitExpression(*t.condition, el);
                       if (t.valueIfTrue)
                           emitExpression(*t.valueIfTrue, el);
                       if (t.valueIfFalse)
                           emitExpression(*t.valueIfFalse, el);
                   },
                   [&](const CallExpression &call) {
                       // The block Vizzy writes for a call it does not know
                       auto appendCustom = [&] {
                           auto el = parent.append_child(
                               "CallCustomExpression");
                           el.append_attribute("call") =
                               call.identifier.c_str();
                           el.append_attribute("style") =
                               "call-custom-expression";
                           for (const auto &p : call.params) {
                               emitExpression(*p, el);
                           }
                       };
                       // Standard calls carry their arguments in source order
                       auto appendParams = [&](pugi::xml_node el) {
                           for (const auto &p : call.params) {
                               emitExpression(*p, el);
                           }
                       };

                       // The coordinate conversions are Vizzy's Planet
                       // blocks even though Juno files them under Nav
                       const PlanetOp *planetOp =
                           call.qualifier == "Planet" ||
                                   call.qualifier == "Nav"
                               ? findPlanetOp(call.identifier)
                               : nullptr;
                       if (planetOp) {
                           auto el = parent.append_child("Planet");
                           el.append_attribute("op") = planetOp->op;
                           el.append_attribute("style") = planetOp->style;
                           appendParams(el);
                       } else if (call.qualifier == "Planet") {
                           appendCustom();
                       } else if (call.qualifier == "ListOp") {
                            if (auto listOp = findListOp(call.identifier)) {
                                auto el = parent.append_child("ListOp");
                                el.append_attribute("op") = listOp->op;
                                el.append_attribute("style") = listOp->style;
                                for (const auto &p : call.params) {
                                    emitExpression(*p, el);
                                }
                            } else {
                                auto el = parent.append_child(
                                    "CallCustomExpression");
                                el.append_attribute("call") =
                                    call.identifier.c_str();
                                el.append_attribute("style") =
                                    "call-custom-expression";
                                for (const auto &p : call.params) {
                                    emitExpression(*p, el);
                                }
                            }
                        } else if (call.qualifier == "Math") {
                            if (auto block =
                                    findMathBlock(call.identifier)) {
                                auto el = parent.append_child(block->tag);
                                if (block->attr) {
                                    el.append_attribute(block->attr) =
                                        block->value;
                                }
                                el.append_attribute("style") = block->style;
                                appendParams(el);
                            } else {
                                appendCustom();
                            }
                        } else if (call.qualifier == "String" ||
                                   call.qualifier == "Strings") {
                            if (auto block = findStringOp(call.identifier)) {
                                auto el = parent.append_child("StringOp");
                                el.append_attribute("op") = block->op;
                                el.append_attribute("style") = block->style;
                                if (call.identifier == "letterOf" &&
                                    call.params.size() == 2) {
                                    // Vizzy reads letter(index, text)
                                    emitExpression(*call.params[1], el);
                                    emitExpression(*call.params[0], el);
                                } else if (call.identifier == "substr" &&
                                           call.params.size() == 3) {
                                    // Vizzy reads substring(start, length,
                                    // text)
                                    emitExpression(*call.params[1], el);
                                    emitExpression(*call.params[2], el);
                                    emitExpression(*call.params[0], el);
                                } else {
                                    appendParams(el);
                                }
                            } else {
                                appendCustom();
                            }
                        } else if (call.qualifier == "Vector") {
                            if (call.identifier == "vec") {
                                // Vizzy builds the vector out of three inputs
                                auto el = parent.append_child("Vector");
                                el.append_attribute("style") = "vec";
                                appendParams(el);
                            } else if (auto block =
                                           findVectorOp(call.identifier)) {
                                auto el = parent.append_child("VectorOp");
                                el.append_attribute("op") = block->op;
                                el.append_attribute("style") = block->style;
                                appendParams(el);
                            } else {
                                appendCustom();
                            }
                        } else if (call.qualifier == "Input" &&
                                   call.identifier == "slider") {
                            // Vizzy has four fixed sliders: the index names
                            // the property instead of being an input
                            const auto *index =
                                call.params.size() == 1
                                    ? std::get_if<Value>(
                                          &call.params[0]->value)
                                    : nullptr;
                            const bool known =
                                index &&
                                index->kind == Value::Kind::Literal &&
                                index->value.size() == 1 &&
                                index->value[0] >= '1' &&
                                index->value[0] <= '4';
                            if (known) {
                                auto el = parent.append_child("CraftProperty");
                                el.append_attribute("property") =
                                    ("Input.Slider" +
                                     std::string(1, index->value[0]))
                                        .c_str();
                                el.append_attribute("style") = "prop-input";
                            } else {
                                appendCustom();
                            }
                        } else if (call.qualifier == "Input" &&
                                   call.identifier == "activationGroup") {
                            auto el = parent.append_child("ActivationGroup");
                            el.append_attribute("style") =
                                "activation-group";
                            appendParams(el);
                        } else if (call.qualifier == "Craft" &&
                                   call.identifier == "name") {
                            // Without an id the name of the current craft,
                            // with one the name of that craft
                            const bool byId = !call.params.empty();
                            auto el = parent.append_child("CraftProperty");
                            el.append_attribute("property") =
                                byId ? "Craft.IDToName" : "Name.Craft";
                            el.append_attribute("style") =
                                byId ? "craft" : "prop-name";
                            appendParams(el);
                        } else if (auto stdExpr = findStdExpr(call.qualifier,
                                                              call.identifier.c_str())) {
                            auto el = parent.append_child("CraftProperty");
                            el.append_attribute("property") =
                                stdExpr->property;
                            el.append_attribute("style") = stdExpr->style;
                            appendParams(el);
                        } else {
                            appendCustom();
                        }
                    },
                },
                expr.value);
}

void Compiler::emitCondition(const Expression &expr, pugi::xml_node parent) {
    // Conditions use the same block vocabulary as any other value: Vizzy
    // stores a comparison as <Comparison>, and/or as <BoolOp>, a bare true
    // as a constant, and so on.
    emitExpression(expr, parent);
}

void Compiler::emitListIndex(const Expression &index,
                             pugi::xml_node parent) {
    // Vizzy reads the first element as 1, Juno writes it as 0, so an
    // integer index can simply be written as the next number
    if (const auto *v = std::get_if<Value>(&index.value);
        v && v->kind == Value::Kind::Literal) {
        try {
            std::size_t used = 0;
            const long long n = std::stoll(v->value, &used);
            if (used == v->value.size()) {
                auto c = parent.append_child("Constant");
                c.append_attribute("number") =
                    std::to_string(n + 1).c_str();
                return;
            }
        } catch (...) {
            // not an integer: use the general form below
        }
    }

    auto bin = parent.append_child("BinaryOp");
    bin.append_attribute("op") = "+";
    bin.append_attribute("style") = "op-add";
    emitExpression(index, bin);
    auto c = bin.append_child("Constant");
    c.append_attribute("number") = "1";
}

std::map<std::string, std::string>
Compiler::getVariables(const RootNode &root) {
    mVariables.clear();
    mVariableOrder.clear();
    mLists.clear();
    mScopePath.clear();
    mUsedNames.clear();

    reserveGlobals(root);

    for (const auto &node : root.body) {
        std::visit(Overload{
                       [this](const EventNode &event) {
                           mScopePath.push_back(event.identifier);
                           collectBody(event.body);
                           mScopePath.pop_back();
                       },
                       [this](const FunctionNode &function) {
                           mScopePath.push_back(function.identifier);
                           collectBody(function.body);
                           mScopePath.pop_back();
                       },
                       [](const auto &) {
                           // imports and top level variables, already reserved;
                           // expressions hold no declarations
                       },
                   },
                   node);
    }

    return mVariables;
}

void Compiler::reserveGlobals(const RootNode &root) {
    for (const auto &node : root.body) {
        std::visit(Overload{
                       [this](const VarDeclaration &varDecl) {
                           mUsedNames.insert(varDecl.identifier);
                           if (mVariables
                                   .emplace(varDecl.identifier,
                                            varDecl.identifier)
                                   .second) {
                               mVariableOrder.push_back(
                                   varDecl.identifier);
                           }
                       },
                       [this](const ListDeclaration &listDecl) {
                           mUsedNames.insert(listDecl.identifier);
                           if (mVariables
                                   .emplace(listDecl.identifier,
                                            listDecl.identifier)
                                   .second) {
                               mVariableOrder.push_back(
                                   listDecl.identifier);
                               mLists.insert(listDecl.identifier);
                           }
                       },
                       [](const auto &) {},
                   },
                   node);
    }
}

void Compiler::declareScoped(const std::string &name, bool isList) {
    std::string key;
    for (const auto &component : mScopePath) {
        key += component;
        key += '.';
    }
    key += name;

    auto emitted = makeUnique(name, key);
    if (mVariables.emplace(key, emitted).second) {
        mVariableOrder.push_back(emitted);
        if (isList)
            mLists.insert(emitted);
    }
}

std::string Compiler::makeUnique(const std::string &name,
                                 const std::string &key) {
    // FNV-1a rather than std::hash: the emitted names have to stay the same
    // across builds and standard libraries.
    std::uint32_t hash = 2166136261u;
    for (unsigned char byte : key) {
        hash ^= byte;
        hash *= 16777619u;
    }

    auto candidate = std::format("{}_{:08x}", name, hash);
    for (unsigned suffix = 1; !mUsedNames.insert(candidate).second; ++suffix) {
        candidate = std::format("{}_{:08x}_{}", name, hash, suffix);
    }
    return candidate;
}

std::string Compiler::resolveVariableName(const std::string &name) {
    // Try from innermost scope outward, keeping the variable name:
    // "start.1.if.a" → "start.1.a" → "start.a" → "a"
    for (std::size_t i = mScopePath.size(); i > 0; --i) {
        std::string key;
        for (std::size_t j = 0; j < i; ++j) {
            key += mScopePath[j];
            key += '.';
        }
        key += name;
        auto it = mVariables.find(key);
        if (it != mVariables.end()) {
            return it->second;
        }
    }
    // Fall back to raw name (global or not yet registered)
    auto it = mVariables.find(name);
    if (it != mVariables.end()) {
        return it->second;
    }
    return name;
}

void Compiler::collectBody(const Body &body) {
    for (std::size_t index = 0; index < body.body.size(); ++index) {
        if (body.body[index]) {
            collectStatement(*body.body[index], index);
        }
    }
}

void Compiler::collectStatement(const Statements &statement,
                                std::size_t index) {
    const auto scope = std::to_string(index);

    std::visit(Overload{
                   [this, scope](const Body &body) {
                       mScopePath.push_back(scope);
                       collectBody(body);
                       mScopePath.pop_back();
                   },
                   [this](const VarDeclaration &varDecl) {
                       declareScoped(varDecl.identifier, false);
                   },
                   [](const VarAssignment &) {
                       // assigns to an existing variable, declares nothing
                   },
                   [this](const ListDeclaration &listDecl) {
                       declareScoped(listDecl.identifier, true);
                   },
                   [](const DoInstruction &) {},
                   [](const BreakStatement &) {},
                   [this, scope](const ConditionalStatement &conditional) {
                       mScopePath.push_back(scope + ".if");
                       collectBody(conditional.ifStatement.body);
                       mScopePath.pop_back();

                       for (std::size_t i = 0;
                            i < conditional.elifStatements.size(); ++i) {
                           mScopePath.push_back(scope + ".elif" +
                                                std::to_string(i));
                           collectBody(conditional.elifStatements[i].body);
                           mScopePath.pop_back();
                       }

                       mScopePath.push_back(scope + ".else");
                       collectBody(conditional.elseStatement);
                       mScopePath.pop_back();
                   },
                   [](const EmitEvent &) {},
                   [this, scope](const RepeatLoop &repeatLoop) {
                       mScopePath.push_back(scope);
                       collectBody(repeatLoop.body);
                       mScopePath.pop_back();
                   },
                   [this, scope](const WhileLoop &whileLoop) {
                       mScopePath.push_back(scope);
                       collectBody(whileLoop.body);
                       mScopePath.pop_back();
                   },
                   [this, scope](const ForLoop &forLoop) {
                       // The iterator and the body share one scope, matching
                       // the semantic analyzer, so a body variable named like
                       // the iterator is a redeclaration rather than a shadow.
                       mScopePath.push_back(scope);
                       declareScoped(forLoop.iterator, false);
                       collectBody(forLoop.body);
                       mScopePath.pop_back();
                   },
               },
                statement.statement);
    }
}
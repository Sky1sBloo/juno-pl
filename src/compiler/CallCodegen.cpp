module;
#include <cctype>
#include <format>
#include <map>
#include <pugixml.hpp>
#include <string>
#include <utility>
#include <variant>
module junopl.compiler;
import junopl.parser.nodes;
import junopl.parser.nodes.expressions;
import junopl.parser.nodes.statements;

namespace JunoPL {
namespace {

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
            {{"Nav", "position"}, {"Nav.Position", "prop-nav"}},
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
            {{"Craft", "pe"}, {"Craft.Periapsis", "craft"}},
            {{"Craft", "timeToAp"}, {"Craft.ApoapsisTime", "craft"}},
            {{"Craft", "timeToPe"}, {"Craft.PeriapsisTime", "craft"}},
            {{"Craft", "eccentricity"}, {"Craft.Eccentricity", "craft"}},
            {{"Craft", "inclination"}, {"Craft.Inclination", "craft"}},
            {{"Craft", "period"}, {"Craft.Period", "craft"}},
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
        {"ap", "apoapsis"},
        {"pe", "periapsis"},
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
    const char *subOp = nullptr;
};

const StringBlock *findStringOp(const std::string &identifier) {
    static const std::map<std::string, StringBlock> kStringOps = {
        {"join", {"join", "join"}},
        {"lengthOf", {"length", "length"}},
        {"letterOf", {"letter", "letter"}},
        {"substr", {"substring", "substring"}},
        {"strContains", {"contains", "contains"}},
        {"format", {"format", "format"}},
        // Human readable values: Vizzy stores the unit in subOp
        {"friendlyAcceleration", {"friendly", "friendly", "acceleration"}},
        {"friendlyAngularVelocity",
         {"friendly", "friendly", "angularvelocity"}},
        {"friendlyCoordinate", {"friendly", "friendly", "coordinate"}},
        {"friendlyDensity", {"friendly", "friendly", "density"}},
        {"friendlyDistance", {"friendly", "friendly", "distance"}},
        {"friendlyEnergy", {"friendly", "friendly", "energy"}},
        {"friendlyForce", {"friendly", "friendly", "force"}},
        {"friendlyIsp", {"friendly", "friendly", "isp"}},
        {"friendlyMass", {"friendly", "friendly", "mass"}},
        {"friendlyPower", {"friendly", "friendly", "power"}},
        {"friendlyPressure", {"friendly", "friendly", "pressure"}},
        {"friendlyTemperature", {"friendly", "friendly", "temperature"}},
        {"friendlyTime", {"friendly", "friendly", "time"}},
        {"friendlyDate", {"friendly", "friendly", "date"}},
        {"friendlyVelocity", {"friendly", "friendly", "velocity"}},
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

} // namespace

void Compiler::emitCallExpression(const CallExpression &call,
                                  pugi::xml_node parent) {
    // The block Vizzy writes for a user defined `expr`
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
         setError(std::format("'Planet.{}' has no implementation", call.identifier));
     } else if (call.qualifier == "ListOp") {
         if (auto listOp = findListOp(call.identifier)) {
             auto el = parent.append_child("ListOp");
             el.append_attribute("op") = listOp->op;
             el.append_attribute("style") = listOp->style;
             for (const auto &p : call.params) {
                 emitExpression(*p, el);
             }
         } else {
             setError(std::format("'ListOp.{}' has no implementation", call.identifier));
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
             setError(std::format("'Math.{}' has no implementation", call.identifier));
         }
     } else if (call.qualifier == "String" ||
                call.qualifier == "Strings") {
         if (auto block = findStringOp(call.identifier)) {
             auto el = parent.append_child("StringOp");
             el.append_attribute("op") = block->op;
             if (block->subOp) {
                 el.append_attribute("subOp") =
                     block->subOp;
             }
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
             setError(std::format("'String.{}' has no implementation", call.identifier));
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
             setError(std::format("'Vector.{}' has no implementation", call.identifier));
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
             setError(std::format("'Input.slider' has no implementation for slider index '{}'", index->value));
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
     } else if (call.qualifier.empty()) {
         // An unqualified call is a user defined `expr`
         appendCustom();
     } else {
         setError(std::format("'{}.{}' has no implementation", call.qualifier, call.identifier));
     }
}

} // namespace JunoPL

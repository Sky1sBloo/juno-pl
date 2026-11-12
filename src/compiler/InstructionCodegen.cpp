module;
#include <cstddef>
#include <format>
#include <functional>
#include <map>
#include <pugixml.hpp>
#include <string>
#include <utility>
#include <variant>
#include "CodegenSupport.hpp"
module junopl.compiler;
import junopl.parser.nodes;
import junopl.parser.nodes.expressions;
import junopl.parser.nodes.statements;

namespace JunoPL {
namespace {

/**
Time mode instructions (e.g. Game.setTimeModeWarp3()) compile to Vizzy
SetTimeMode blocks. The Juno name omits the duplicated "Time", but the
game attribute keeps it: setTimeModeWarp3 -> mode="TimeWarp3".
*/
const char *findTimeMode(const std::string &identifier) {
    static const std::map<std::string, const char *> kTimeModes = {
        {"setTimeModeNormal", "Normal"},
        {"setTimeModePaused", "Paused"},
        {"setTimeModeSlowMotion", "SlowMotion"},
        {"setTimeModeFastForward", "FastForward"},
        {"setTimeModeWarp1", "TimeWarp1"},
        {"setTimeModeWarp2", "TimeWarp2"},
        {"setTimeModeWarp3", "TimeWarp3"},
        {"setTimeModeWarp4", "TimeWarp4"},
        {"setTimeModeWarp5", "TimeWarp5"},
        {"setTimeModeWarp6", "TimeWarp6"},
        {"setTimeModeWarp7", "TimeWarp7"},
        {"setTimeModeWarp8", "TimeWarp8"},
        {"setTimeModeWarp9", "TimeWarp9"},
        {"setTimeModeWarp10", "TimeWarp10"},
    };
    auto it = kTimeModes.find(identifier);
    if (it == kTimeModes.end()) {
        return nullptr;
    }
    return it->second;
}

} // namespace

void Compiler::emitDoInstruction(const DoInstruction &instr,
                                 pugi::xml_node parent,
                                 std::function<std::string()> nextId) {
    if (instr.qualifier.empty() && instr.isInline) {
        auto it = mFunctions.find(instr.identifier);
        if (it == mFunctions.end()) {
            setError(std::format("'{}' is not defined", instr.identifier));
            return;
        }
        const FunctionNode &func = *it->second;

        auto saved = mInlineSubstitutions;
        mInlineSubstitutions.clear();
        for (std::size_t i = 0;
             i < func.params.size() && i < instr.params.size();
             ++i) {
            mInlineSubstitutions.emplace(func.params[i],
                                         instr.params[i].get());
        }
        emitBody(func.body, parent, nextId);
        mInlineSubstitutions = std::move(saved);
        return;
    }

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
        } else if (instr.identifier == "localLog") {
            auto el = parent.append_child("LogMessage");
            el.append_attribute("id") = nextId().c_str();
            el.append_attribute("style") = "log";
            for (const auto &p : instr.params) {
                emitExpression(*p, el);
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
        } else if (const char *mode =
                       findTimeMode(instr.identifier)) {
            auto el = parent.append_child("SetTimeMode");
            el.append_attribute("mode") = mode;
            el.append_attribute("id") = nextId().c_str();
            el.append_attribute("style") = "set-time-mode";
        } else {
            setError(std::format("'Game.{}' has no implementation", instr.identifier));
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
            setError(std::format("'Lists.{}' has no implementation", instr.identifier));
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
                {"fuelTransfer", "Part.FuelTransfer"},
            };
        const auto property =
            kPartProperties.find(instr.identifier);
        if (property == kPartProperties.end()) {
            setError(std::format("'Part.{}' has no implementation", instr.identifier));
            return;
        }
        auto el = parent.append_child("SetCraftProperty");
        el.append_attribute("property") = property->second.c_str();
        el.append_attribute("id") = nextId().c_str();
        el.append_attribute("style") = "set-part";
        for (const auto &p : instr.params) {
            emitExpression(*p, el);
        }
        // Exploding without a strength is a detach
        if (instr.identifier == "explode" &&
            instr.params.size() == 1) {
            auto c = el.append_child("Constant");
            c.append_attribute("number") = "0";
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
            setError(std::format("'{}.{}' has no implementation", instr.qualifier, instr.identifier));
        }
    }
}

} // namespace JunoPL

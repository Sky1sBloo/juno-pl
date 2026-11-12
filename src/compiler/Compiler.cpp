module;
#include <cstddef>
#include <expected>
#include <filesystem>
#include <format>
#include <functional>
#include <map>
#include <optional>
#include <pugixml.hpp>
#include <set>
#include <sstream>
#include <string>
#include <variant>
#include <vector>
#include "CodegenSupport.hpp"
module junopl.compiler;
import junopl.filehandler;
import junopl.parser.nodes;
import junopl.parser.nodes.expressions;
import junopl.parser.nodes.statements;

namespace JunoPL {
namespace {

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

constexpr int kBlockHeight = 30;
constexpr int kThreadGap = 30;

/** Recursively counts every block element a thread will emit, including
 * blocks nested inside conditionals, loops, and other containers. */
std::size_t countTopLevelInitBlocks(const RootNode &root) {
    std::size_t count = 0;
    for (const auto &node : root.body) {
        std::visit(
            Overload{
                [&](const VarDeclaration &vd) {
                    if (vd.value)
                        ++count;
                },
                [&](const ListDeclaration &ld) {
                    if (!ld.values.empty())
                        ++count;
                },
                [](const auto &) {},
            },
            node);
    }
    return count;
}

/** Recursively counts every block element a thread will emit, including
 * blocks nested inside conditionals, loops, and other containers. */
std::size_t countThreadBlocks(const Body &body) {
    std::size_t count = 0;
    for (const auto &stmt : body.body) {
        if (!stmt) continue;
        std::visit(
            Overload{
                [&](const Body &nested) {
                    count += countThreadBlocks(nested);
                },
                [&](const VarDeclaration &) { ++count; },
                [&](const VarAssignment &) { ++count; },
                [&](const ListDeclaration &) { ++count; },
                [&](const DoInstruction &) { ++count; },
                [&](const BreakStatement &) { ++count; },
                [&](const ConditionalStatement &cond) {
                    ++count; // the If block itself
                    count += countThreadBlocks(cond.ifStatement.body);
                    for (const auto &elif : cond.elifStatements)
                        count += countThreadBlocks(elif.body);
                    count += countThreadBlocks(cond.elseStatement);
                },
                [&](const EmitEvent &) { ++count; },
                [&](const RepeatLoop &repeat) {
                    ++count; // the Repeat block itself
                    count += countThreadBlocks(repeat.body);
                },
                [&](const WhileLoop &loop) {
                    ++count; // the While block itself
                    count += countThreadBlocks(loop.body);
                },
                [&](const ForLoop &loop) {
                    ++count; // the For block itself
                    count += countThreadBlocks(loop.body);
                },
            },
            stmt->statement);
    }
    return count;
}

std::string Compiler::nextThreadPos() {
    // One row per thread, every thread starting on the same canvas column,
    // stacking independent event handlers vertically. The y position is
    // advanced by advanceThreadPos() after each thread's body is emitted,
    // so taller threads (e.g. on start with many blocks underneath) push
    // subsequent threads lower.
    return std::format("-10,{}", mThreadY);
}

void Compiler::advanceThreadPos(std::size_t blockCount) {
    mThreadY += static_cast<int>(blockCount * kBlockHeight + kThreadGap);
}

void Compiler::setError(const std::string &message) {
    if (!mError.has_value()) {
        mError = message;
    }
}

std::expected<std::string, std::string> Compiler::compileAST(const RootNode &root) {
    mThreadY = -20;
    mHandlerParams.clear();
    mInlineSubstitutions.clear();
    mError = std::nullopt;

    pugi::xml_document doc;
    auto prog = doc.append_child("Program");
    prog.append_attribute("name") = root.programName.c_str();

    // Build a lookup of user instr definitions for inline calls 
    mFunctions.clear();
    for (const auto &node : root.body) {
        if (const auto *func = std::get_if<FunctionNode>(&node)) {
            mFunctions.emplace(func->identifier, func);
        }
    }

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
                    advanceThreadPos(1 + countThreadBlocks(event.body) +
                                     (isStart ? countTopLevelInitBlocks(root)
                                              : 0));
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
                    advanceThreadPos(1 + countThreadBlocks(func.body));
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
        advanceThreadPos(1 + countTopLevelInitBlocks(root));
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
                    advanceThreadPos(1);
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

    if (mError.has_value()) {
        return std::unexpected(*mError);
    }
    return xml;
}

std::expected<void, FileHandlerError> Compiler::writeToFile(
    const std::filesystem::path &path, const std::string &xml) {
    return FileHandler::writeToFile(path, xml);
}

void Compiler::emitBody(const Body &body, pugi::xml_node parent,
                        std::function<std::string()> nextId) {
    if (mError.has_value()) {
        return;
    }
    for (std::size_t index = 0; index < body.body.size(); ++index) {
        if (body.body[index]) {
            emitStatement(*body.body[index], index, parent, nextId);
        }
    }
}

bool Compiler::emitTopLevelInits(const RootNode &root,
                                 pugi::xml_node parent,
                                 std::function<std::string()> nextId) {
    if (mError.has_value()) {
        return false;
    }
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

} // namespace JunoPL

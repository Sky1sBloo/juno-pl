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
        return "op-ne";
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

const char *comparisonOp(BinaryOp::Operation o) {
    switch (o) {
    case BinaryOp::Operation::EQUAL:
        return "=";
    case BinaryOp::Operation::NOT_EQUAL:
        return "!=";
    case BinaryOp::Operation::GREATER:
        return "g";
    case BinaryOp::Operation::LESS:
        return "l";
    case BinaryOp::Operation::GREATER_EQ:
        return ">=";
    case BinaryOp::Operation::LESS_EQ:
        return "<=";
    }
    return "";
}

std::pair<std::string, std::string> eventName(const std::string &junoEvent) {
    if (junoEvent == "start")
        return {"FlightStart", "flight-start"};
    if (junoEvent == "collision")
        return {"PartCollision", "part-collision"};
    if (junoEvent == "dock")
        return {"Docked", "craft-docked"};
    if (junoEvent == "explode")
        return {"PartExplode", "part-explode"};
    if (junoEvent == "enterSOI")
        return {"ChangeSoi", "change-soi"};
    if (junoEvent == "receiveMessage")
        return {"ReceiveMessage", "receive-msg"};
    // Unknown event: passthrough with kebab-style
    std::string style = junoEvent;
    for (auto &c : style)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return {junoEvent, style};
}

} // namespace

std::string Compiler::compileAST(const RootNode &root) {
    pugi::xml_document doc;
    auto prog = doc.append_child("Program");
    prog.append_attribute("name") = root.programName.c_str();

    // ---- Variables ----
    auto varsEl = prog.append_child("Variables");
    for (const auto &[key, name] : mVariables) {
        auto v = varsEl.append_child("Variable");
        v.append_attribute("name") = name.c_str();
        v.append_attribute("number") = "0";
    }

    // ---- Instructions ----
    auto instrsEl = prog.append_child("Instructions");
    std::size_t id = 0;
    auto nextId = [&id] { return std::to_string(id++); };

    for (const auto &node : root.body) {
        std::visit(
            Overload{
                [&](const EventNode &event) {
                    auto [xmlEvent, xmlStyle] = eventName(event.identifier);
                    auto el = instrsEl.append_child("Event");
                    el.append_attribute("event") = xmlEvent.c_str();
                    el.append_attribute("id") = nextId().c_str();
                    el.append_attribute("style") = xmlStyle.c_str();
                    el.append_attribute("pos") = "-10,-20";

                    if (!event.params.empty()) {
                        for (const auto &p : event.params) {
                            auto pn = el.append_child("Parameter");
                            pn.append_attribute("name") = p.c_str();
                        }
                    }
                    mScopePath.push_back(event.identifier);
                    emitBody(event.body, instrsEl, nextId);
                    mScopePath.pop_back();
                },
                [&](const FunctionNode &func) {
                    auto el = instrsEl.append_child("CustomInstruction");
                    el.append_attribute("id") = nextId().c_str();
                    el.append_attribute("name") = func.identifier.c_str();
                    el.append_attribute("style") = "custom-instruction";
                    std::string paramsStr;
                    for (std::size_t k = 0; k < func.params.size(); ++k) {
                        if (k)
                            paramsStr += " ";
                        paramsStr += "|" + func.params[k] + "|";
                    }
                    el.append_attribute("format") =
                        (func.identifier + " " + paramsStr + "  ").c_str();
                    std::string callFormat;
                    for (std::size_t k = 0; k < func.params.size(); ++k) {
                        if (k)
                            callFormat += " ";
                        callFormat += "(" + std::to_string(k) + ")";
                    }
                    el.append_attribute("callFormat") =
                        (func.identifier + " " + callFormat + " ").c_str();
                    mScopePath.push_back(func.identifier);
                    emitBody(func.body, instrsEl, nextId);
                    mScopePath.pop_back();
                },
                [](const auto &) {
                    // imports, top-level vars, custom expressions
                    // handled elsewhere
                },
            },
            node);
    }

    // ---- Expressions ----
    (void)prog.append_child("Expressions");
    std::ostringstream oss;
    pugi::xml_writer_stream writer(oss);
    doc.save(writer, PUGIXML_TEXT("\t"), pugi::format_default, pugi::encoding_utf8);
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
            },
            [&](const VarAssignment &assign) {
                auto el = parent.append_child("SetVariable");
                el.append_attribute("id") = nextId().c_str();
                el.append_attribute("style") = "set-variable";
                auto ref = el.append_child("Variable");
                ref.append_attribute("list") = "false";
                ref.append_attribute("local") = "false";
                ref.append_attribute("variableName") =
                    resolveVariableName(assign.identifier).c_str();
                if (assign.value) {
                    emitExpression(*assign.value, el);
                } else {
                    auto c = el.append_child("Constant");
                    c.append_attribute("number") = "0";
                }
            },
            [&](const ListDeclaration &listDecl) {
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
                auto c = listOp.append_child("Constant");
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
                    c.append_attribute("text") = csv.c_str();
                }
            },
            [&](const PerformInstruction &instr) {
                if (instr.qualifier == "Game" && instr.identifier == "display") {
                    auto el = parent.append_child("DisplayMessage");
                    el.append_attribute("id") = nextId().c_str();
                    el.append_attribute("style") = "display";
                    for (const auto &p : instr.params) {
                        emitExpression(*p, el);
                    }
                } else {
                    auto el = parent.append_child("SetCraftProperty");
                    el.append_attribute("id") = nextId().c_str();
                    el.append_attribute("style") = "set-part";
                    el.append_attribute("property") = instr.identifier.c_str();
                    for (const auto &p : instr.params) {
                        emitExpression(*p, el);
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

                auto elseEl = parent.append_child("ElseIf");
                elseEl.append_attribute("id") = nextId().c_str();
                elseEl.append_attribute("style") = "else";
                auto c = elseEl.append_child("Constant");
                c.append_attribute("style") = "true";
                c.append_attribute("bool") = "true";
                mScopePath.push_back(std::to_string(index) + ".else");
                auto elseBody = elseEl.append_child("Instructions");
                emitBody(cond.elseStatement, elseBody, nextId);
                mScopePath.pop_back();
            },
            [&](const EmitEvent &emit) {
                auto el = parent.append_child("BroadcastMessage");
                el.append_attribute("id") = nextId().c_str();
                el.append_attribute("style") = "broadcast-msg";
                el.append_attribute("message") = emit.identifier.c_str();
                el.append_attribute("global") =
                    (emit.option == EmitEvent::TO_NEARBY_CRAFTS) ? "true"
                                                                 : "false";
                el.append_attribute("local") =
                    (emit.option == EmitEvent::TO_SCRIPT) ? "true" : "false";
                auto mc = el.append_child("Constant");
                mc.append_attribute("text") = emit.identifier.c_str();
                for (const auto &p : emit.params) {
                    emitExpression(*p, el);
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
                    countingDown ? ">=" : "<=";
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
                           auto el = parent.append_child("Constant");
                           try {
                               std::stod(v.value);
                               el.append_attribute("number") = v.value.c_str();
                           } catch (...) {
                               el.append_attribute("text") = v.value.c_str();
                           }
                    } else {
                        auto ref = parent.append_child("Variable");
                        ref.append_attribute("list") = "false";
                        ref.append_attribute("local") = "false";
                        ref.append_attribute("variableName") =
                            resolveVariableName(v.value).c_str();
                    }
                   },
                   [&](const BinaryOp &bop) {
                       auto el = parent.append_child("BinaryOp");
                       el.append_attribute("op") = binopOp(bop.operation);
                       el.append_attribute("style") = binopStyle(bop.operation);
                       if (bop.lhs)
                           emitExpression(*bop.lhs, el);
                       if (bop.rhs)
                           emitExpression(*bop.rhs, el);
                   },
                    [&](const UnaryOp &uop) {
                        if (uop.operation == UnaryOp::Operation::NEGATE &&
                            uop.value &&
                            std::holds_alternative<Value>(uop.value->value)) {
                            const Value &v =
                                std::get<Value>(uop.value->value);
                            auto el = parent.append_child("Constant");
                            if (v.kind == Value::Kind::Literal) {
                                try {
                                    double d = std::stod(v.value);
                                    double neg = -d;
                                    std::string numStr =
                                        std::to_string(neg);
                                    // Strip trailing zeros after decimal point
                                    if (numStr.find('.') !=
                                        std::string::npos) {
                                        numStr.erase(
                                            numStr.find_last_not_of('0') +
                                            1, std::string::npos);
                                        if (numStr.back() == '.') {
                                            numStr.pop_back();
                                        }
                                    }
                                    el.append_attribute("number") =
                                        numStr.c_str();
                                } catch (...) {
                                    el.append_attribute("text") =
                                        ("-" + v.value).c_str();
                                }
                            } else {
                                el.append_attribute("text") =
                                    ("-" + v.value).c_str();
                            }
                        } else if (uop.operation ==
                                   UnaryOp::Operation::NEGATE) {
                            // 0 - value for non-literal negate
                            auto el = parent.append_child("BinaryOp");
                            el.append_attribute("op") = "-";
                            el.append_attribute("style") = "op-sub";
                            auto c = el.append_child("Constant");
                            c.append_attribute("number") = "0";
                            if (uop.value)
                                emitExpression(*uop.value, el);
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
                           emitExpression(*li.index, el);
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
                       auto el = parent.append_child("CallCustomExpression");
                       el.append_attribute("call") = call.identifier.c_str();
                       el.append_attribute("style") = "call-custom-expression";
                       for (const auto &p : call.params) {
                           emitExpression(*p, el);
                       }
                   },
                },
                expr.value);
}

void Compiler::emitCondition(const Expression &expr, pugi::xml_node parent) {
    if (const auto *bop = std::get_if<BinaryOp>(&expr.value)) {
        auto el = parent.append_child("Comparison");
        el.append_attribute("op") = comparisonOp(bop->operation);
        el.append_attribute("style") = binopStyle(bop->operation);
        if (bop->lhs)
            emitExpression(*bop->lhs, el);
        if (bop->rhs)
            emitExpression(*bop->rhs, el);
    } else {
        emitExpression(expr, parent);
    }
}

std::map<std::string, std::string>
Compiler::getVariables(const RootNode &root) {
    mVariables.clear();
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
                           mVariables.emplace(varDecl.identifier,
                                              varDecl.identifier);
                           mUsedNames.insert(varDecl.identifier);
                       },
                       [this](const ListDeclaration &listDecl) {
                           mVariables.emplace(listDecl.identifier,
                                              listDecl.identifier);
                           mUsedNames.insert(listDecl.identifier);
                       },
                       [](const auto &) {},
                   },
                   node);
    }
}

void Compiler::declareScoped(const std::string &name) {
    std::string key;
    for (const auto &component : mScopePath) {
        key += component;
        key += '.';
    }
    key += name;

    mVariables.emplace(key, makeUnique(name, key));
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
                       declareScoped(varDecl.identifier);
                   },
                   [](const VarAssignment &) {
                       // assigns to an existing variable, declares nothing
                   },
                   [this](const ListDeclaration &listDecl) {
                       declareScoped(listDecl.identifier);
                   },
                   [](const PerformInstruction &) {},
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
                       declareScoped(forLoop.iterator);
                       collectBody(forLoop.body);
                       mScopePath.pop_back();
                   },
               },
                statement.statement);
    }
}
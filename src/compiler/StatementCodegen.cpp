module;
#include <cstddef>
#include <functional>
#include <pugixml.hpp>
#include <string>
#include <variant>
#include "CodegenSupport.hpp"
module junopl.compiler;
import junopl.parser.nodes;
import junopl.parser.nodes.expressions;
import junopl.parser.nodes.statements;

namespace JunoPL {

void Compiler::emitStatement(const Statements &statement, std::size_t index,
                             pugi::xml_node parent,
                             std::function<std::string()> nextId) {
    if (mError.has_value()) {
        return;
    }
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
                emitVarAssignment(assign, parent, nextId);
            },
            [&](const ListDeclaration &listDecl) {
                emitListDeclaration(listDecl, parent, nextId);
            },
            [&](const DoInstruction &instr) {
                emitDoInstruction(instr, parent, nextId);
            },
            [&](const BreakStatement &) {
                auto el = parent.append_child("Break");
                el.append_attribute("id") = nextId().c_str();
                el.append_attribute("style") = "break";
            },
            [&](const ConditionalStatement &cond) {
                emitConditionalStatement(cond, index, parent, nextId);
            },
            [&](const EmitEvent &emit) {
                emitEmitEvent(emit, parent, nextId);
            },
            [&](const RepeatLoop &repeat) {
                emitRepeatLoop(repeat, index, parent, nextId);
            },
            [&](const WhileLoop &loop) {
                emitWhileLoop(loop, index, parent, nextId);
            },
            [&](const ForLoop &loop) {
                emitForLoop(loop, index, parent, nextId);
            },
        },
        statement.statement);
}

void Compiler::emitConditionalStatement(const ConditionalStatement &cond,
                                        std::size_t index,
                                        pugi::xml_node parent,
                                        std::function<std::string()> nextId) {
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
}

void Compiler::emitEmitEvent(const EmitEvent &emit, pugi::xml_node parent,
                             std::function<std::string()> nextId) {
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
}

void Compiler::emitRepeatLoop(const RepeatLoop &repeat, std::size_t index,
                              pugi::xml_node parent,
                              std::function<std::string()> nextId) {
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
}

void Compiler::emitWhileLoop(const WhileLoop &loop, std::size_t index,
                             pugi::xml_node parent,
                             std::function<std::string()> nextId) {
    auto el = parent.append_child("While");
    el.append_attribute("id") = nextId().c_str();
    el.append_attribute("style") = "while";
    emitCondition(*loop.condition, el);
    mScopePath.push_back(std::to_string(index));
    auto bodyEl = el.append_child("Instructions");
    emitBody(loop.body, bodyEl, nextId);
    mScopePath.pop_back();
}

void Compiler::emitForLoop(const ForLoop &loop, std::size_t index,
                           pugi::xml_node parent,
                           std::function<std::string()> nextId) {
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
}

} // namespace JunoPL

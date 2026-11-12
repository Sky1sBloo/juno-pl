module;
#include <cstddef>
#include <pugixml.hpp>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include "CodegenSupport.hpp"
module junopl.compiler;
import junopl.parser.nodes;
import junopl.parser.nodes.expressions;
import junopl.parser.nodes.statements;

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

} // namespace

void Compiler::emitExpression(const Expression &expr, pugi::xml_node parent) {
    if (mError.has_value()) {
        return;
    }
    std::visit(Overload{
                   [&](const Value &v) { emitValue(v, parent); },
                   [&](const BinaryOp &bop) { emitBinaryOp(bop, parent); },
                   [&](const UnaryOp &uop) { emitUnaryOp(uop, parent); },
                   [&](const ListIndex &li) {
                       emitListIndexGet(li, parent);
                   },
                   [&](const TernaryOp &t) { emitTernary(t, parent); },
                   [&](const CallExpression &call) {
                       emitCallExpression(call, parent);
                   },
               },
               expr.value);
}

void Compiler::emitValue(const Value &v, pugi::xml_node parent) {
    if (v.kind == Value::Kind::Literal) {
        appendConstant(parent, v.value);
        return;
    }
     if (v.kind == Value::Kind::String) {
         appendStringConstant(parent, v.value);
         return;
     }

     // Inline substitution: replace a formal parameter with
     // the argument expression from the call site.
     if (v.kind == Value::Kind::Identifier) {
         auto sub = mInlineSubstitutions.find(v.value);
         if (sub != mInlineSubstitutions.end()) {
             emitExpression(*sub->second, parent);
             return;
         }
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
}

void Compiler::emitBinaryOp(const BinaryOp &bop, pugi::xml_node parent) {
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
}

void Compiler::emitUnaryOp(const UnaryOp &uop, pugi::xml_node parent) {
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
}

void Compiler::emitListIndexGet(const ListIndex &li,
                                pugi::xml_node parent) {
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
}

void Compiler::emitTernary(const TernaryOp &t, pugi::xml_node parent) {
    auto el = parent.append_child("Conditional");
    el.append_attribute("style") = "conditional";
    if (t.condition)
        emitExpression(*t.condition, el);
    if (t.valueIfTrue)
        emitExpression(*t.valueIfTrue, el);
    if (t.valueIfFalse)
        emitExpression(*t.valueIfFalse, el);
}

void Compiler::emitCondition(const Expression &expr, pugi::xml_node parent) {
    // Conditions use the same block vocabulary as any other value: Vizzy
    // stores a comparison as <Comparison>, and/or as <BoolOp>, a bare true
    // as a constant, and so on.
    emitExpression(expr, parent);
}

} // namespace JunoPL

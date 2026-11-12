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

void Compiler::emitVarDeclaration(const VarDeclaration &varDecl,
                                  pugi::xml_node parent,
                                  std::function<std::string()> nextId) {
    if (mError.has_value()) {
        return;
    }
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
    if (mError.has_value()) {
        return;
    }
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

void Compiler::emitVarAssignment(const VarAssignment &assign,
                                 pugi::xml_node parent,
                                 std::function<std::string()> nextId) {
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

} // namespace JunoPL

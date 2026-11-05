#include "doctest.h"
#include <map>
#include <set>
#include <string>

import junopl.compiler;
import junopl.lexer;
import junopl.parser;
import junopl.parser.nodes;
import junopl.tests.filehandler;

namespace {
std::map<std::string, std::string> variablesOf(const std::string &source) {
    JunoPL::Tests::FileHandlerTest fileHandler;
    fileHandler.setSourceCode(source);

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileHandler);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);
    REQUIRE(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    JunoPL::Compiler compiler;
    return compiler.getVariables(*parseOutput.root);
}

/** Name a scoped variable got, failing the test when it is missing. */
const std::string &scoped(const std::map<std::string, std::string> &variables,
                          const std::string &key) {
    REQUIRE(variables.count(key) == 1);
    return variables.at(key);
}
} // namespace

TEST_CASE("Compiler - top level variables keep their names") {
    const auto variables =
        variablesOf("program test; var top = 1; list nums = [1, 2];");

    REQUIRE(variables.size() == 2);
    CHECK(scoped(variables, "top") == "top");
    CHECK(scoped(variables, "nums") == "nums");
}

TEST_CASE("Compiler - scoped variables get a unique readable name") {
    const auto variables =
        variablesOf("program test; instr main() { var x = 1; }");

    REQUIRE(variables.size() == 1);
    const auto &name = scoped(variables, "main.x");
    CHECK(name.starts_with("x_"));
    CHECK(name != "x");
    CHECK(name.size() == std::string("x_").size() + 8);
    CHECK(variables.count("x") == 0);
}

TEST_CASE("Compiler - same name in different functions stays unique") {
    const auto variables = variablesOf(
        "program test; instr a() { var x = 1; } instr b() { var x = 2; }");

    REQUIRE(variables.size() == 2);
    CHECK(scoped(variables, "a.x").starts_with("x_"));
    CHECK(scoped(variables, "b.x").starts_with("x_"));
    CHECK(variables.at("a.x") != variables.at("b.x"));
}

TEST_CASE("Compiler - a global and a local of the same name differ") {
    const auto variables =
        variablesOf("program test; var x = 0; instr main() { var x = 1; }");

    REQUIRE(variables.size() == 2);
    CHECK(scoped(variables, "x") == "x");
    CHECK(variables.at("main.x") != "x");
    CHECK(variables.at("main.x").starts_with("x_"));
}

TEST_CASE("Compiler - a shadowed variable gets its own name") {
    const auto variables = variablesOf(
        "program test; instr main() { var x = 1; if (x) { var x = 2; } }");

    REQUIRE(variables.size() == 2);
    CHECK(scoped(variables, "main.x").starts_with("x_"));
    CHECK(scoped(variables, "main.1.if.x").starts_with("x_"));
    CHECK(variables.at("main.x") != variables.at("main.1.if.x"));
}

TEST_CASE("Compiler - sibling blocks declaring the same name differ") {
    const auto variables =
        variablesOf("program test; instr main() { if (true) { var x = 1; } "
                    "while (true) { var x = 2; } }");

    REQUIRE(variables.size() == 2);
    CHECK(scoped(variables, "main.0.if.x").starts_with("x_"));
    CHECK(scoped(variables, "main.1.x").starts_with("x_"));
    CHECK(variables.at("main.0.if.x") != variables.at("main.1.x"));
}

TEST_CASE("Compiler - event, branch and loop scopes are in the key path") {
    const auto variables = variablesOf(
        "program test; on start() { var a = 1; if (true) { var b = 2; } "
        "elif (false) { var c = 3; } else { var d = 4; } "
        "for i from 0 to 5 by 1 { var e = i; } }");

    CHECK(scoped(variables, "start.a").starts_with("a_"));
    CHECK(scoped(variables, "start.1.if.b").starts_with("b_"));
    CHECK(scoped(variables, "start.1.elif0.c").starts_with("c_"));
    CHECK(scoped(variables, "start.1.else.d").starts_with("d_"));
    // The for loop is statement 2 and its iterator shares the body's scope.
    CHECK(scoped(variables, "start.2.i").starts_with("i_"));
    CHECK(scoped(variables, "start.2.e").starts_with("e_"));
}

TEST_CASE("Compiler - for loop iterator is collected and readable") {
    const auto variables = variablesOf("program test; instr main() { for i "
                                       "from 0 to 10 by 1 { var y = i; } }");

    REQUIRE(variables.size() == 2);
    const auto &iterator = scoped(variables, "main.0.i");
    CHECK(iterator.starts_with("i_"));
    CHECK(iterator != "i");
    CHECK(scoped(variables, "main.0.y").starts_with("y_"));
    CHECK(iterator != variables.at("main.0.y"));
}

TEST_CASE("Compiler - emitted names and keys are free of separators") {
    const auto variables =
        variablesOf("program test; var g = 1; on start() { var a = 1; "
                    "if (true) { var b = 2; } else { var c = 3; } "
                    "for i from 0 to 3 by 1 { var d = i; } } "
                    "instr main() { while (true) { var e = 1; } }");

    REQUIRE(variables.size() == 7);
    std::set<std::string> names;
    for (const auto &[key, name] : variables) {
        // A key may only separate components with '.', never '/'.
        CHECK(key.find('/') == std::string::npos);
        // Only the value is emitted as a variable name, so it must be a
        // plain identifier: no '.' and no '/'.
        CHECK(name.find('/') == std::string::npos);
        CHECK(name.find('.') == std::string::npos);
        // Every variable ends up under its own global name.
        CHECK(names.insert(name).second);
    }
}

TEST_CASE("Compiler - a global may not steal a generated name") {
    const auto plain =
        variablesOf("program test; var x = 0; instr main() { var x = 1; }");
    REQUIRE(plain.count("main.x") == 1);
    const auto generated = plain.at("main.x");

    // The same program with a top level variable already using the name the
    // local would have generated: the global keeps it and the local gains a
    // counter instead of silently sharing it.
    const auto clashing =
        variablesOf("program test; var x = 0; var " + generated +
                    " = 9; instr main() { var x = 1; }");
    REQUIRE(clashing.count("x") == 1);
    REQUIRE(clashing.count(generated) == 1);
    CHECK(clashing.at("x") == "x");
    CHECK(clashing.at(generated) == generated);
    CHECK(clashing.at("main.x") == generated + "_1");
}

TEST_CASE("Compiler - getVariables is deterministic") {
    const std::string source =
        "program test; var g = 1; on start() { var a = 1; "
        "if (true) { var b = 2; } for i from 0 to 3 by 1 { var c = i; } } "
        "instr main() { while (true) { var d = 1; } }";

    CHECK(variablesOf(source) == variablesOf(source));
}

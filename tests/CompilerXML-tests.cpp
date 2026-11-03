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

std::string compileSource(const std::string &source) {
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
    compiler.getVariables(*parseOutput.root);
    return compiler.compileAST(*parseOutput.root);
}

std::string variablesOf(const std::string &source) {
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
    auto variables = compiler.getVariables(*parseOutput.root);
    std::string result;
    for (const auto &[key, name] : variables) {
        if (!result.empty())
            result += ",";
        result += key + "=" + name;
    }
    return result;
}

} // namespace

TEST_CASE("Compiler XML - hello world matches Vizzy format") {
    const auto xml = compileSource(R"(
program test;
var Test1 = 0;
var Test2 = 0;

on start() {
    perform Game.display("Hello world", 7);
    Test1 = 1;
})");

    CHECK(xml.find("<?xml version=\"1.0\" encoding=\"utf-8\"?>") !=
          std::string::npos);
    CHECK(xml.find("<Program name=\"test\">") != std::string::npos);
    CHECK(xml.find("<Variables>") != std::string::npos);
    CHECK(xml.find("<Variable name=\"Test1\" number=\"0\" />") !=
          std::string::npos);
    CHECK(xml.find("<Variable name=\"Test2\" number=\"0\" />") !=
          std::string::npos);
    CHECK(xml.find("</Variables>") != std::string::npos);
    CHECK(xml.find("<Instructions>") != std::string::npos);
    // Event is self-closing, statements are siblings
    CHECK(xml.find("<Event event=\"FlightStart\" id=\"0\" style=\"flight-start\" "
                   "pos=\"-10,-20\" />") != std::string::npos);
    CHECK(xml.find("<DisplayMessage id=\"1\" style=\"display\">") !=
          std::string::npos);
    CHECK(xml.find("<Constant text=\"Hello world\" />") != std::string::npos);
    CHECK(xml.find("<Constant number=\"7\" />") != std::string::npos);
    CHECK(xml.find("</DisplayMessage>") != std::string::npos);
    CHECK(xml.find("</Instructions>") != std::string::npos);
    CHECK(xml.find("<Expressions />") != std::string::npos);
    CHECK(xml.find("</Program>") != std::string::npos);
}

TEST_CASE("Compiler XML - event start maps to FlightStart") {
    const auto xml = compileSource(R"(
program test;
on start() {
    var x = 1;
})");

    CHECK(xml.find("<Event event=\"FlightStart\" id=\"0\" style=\"flight-start\" "
                   "pos=\"-10,-20\" />") != std::string::npos);
}

TEST_CASE("Compiler XML - unary minus folds into constant") {
    const auto xml = compileSource(R"(
program test;
on start() {
    var x = -4;
    perform Game.display(x);
})");

    CHECK(xml.find("<Constant number=\"-4\" />") != std::string::npos);
    CHECK(xml.find("<Constant number=\"4\" />") == std::string::npos);
}

TEST_CASE("Compiler XML - unary minus on variable emits BinaryOp") {
    const auto xml = compileSource(R"(
program test;
var y = 10;
on start() {
    var x = -y;
})");

    CHECK(xml.find("<BinaryOp op=\"-\" style=\"op-sub\">") != std::string::npos);
    CHECK(xml.find("<Constant number=\"0\" />") != std::string::npos);
}

TEST_CASE("Compiler XML - for loop lowers to while loop") {
    const auto xml = compileSource(R"(
program test;
var i = 0;
var x = 0;

on start() {
    for i from 0 to 2 by 1 {
        x = i;
    }
})");

    // No <For> element should exist
    CHECK(xml.find("<For ") == std::string::npos);
    CHECK(xml.find("</For>") == std::string::npos);

    // Init: i = 0 (uses hashed name since i is scoped)
    CHECK(xml.find("variableName=\"i_") != std::string::npos);

    // While condition: i <= 2
    CHECK(xml.find("<Comparison op=\"&lt;=\" style=\"op-lte\">") !=
          std::string::npos);
    CHECK(xml.find("<Constant number=\"2\" />") != std::string::npos);

    // Increment: i = i + 1
    CHECK(xml.find("<BinaryOp op=\"+\" style=\"op-add\">") !=
          std::string::npos);
    CHECK(xml.find("<Constant number=\"1\" />") != std::string::npos);
}

TEST_CASE("Compiler XML - for loop with negative step uses >= condition") {
    const auto xml = compileSource(R"(
program test;
var i = 0;
var x = 0;

on start() {
    for i from 3 to 0 by -1 {
        x = x + i;
    }
})");

    CHECK(xml.find("<For ") == std::string::npos);
    CHECK(xml.find("</For>") == std::string::npos);

    // Condition: i >= 0 (negative step)
    CHECK(xml.find("<Comparison op=\">=\" style=\"op-gte\">") !=
          std::string::npos);
    CHECK(xml.find("<Constant number=\"0\" />") != std::string::npos);

    // Init: i = 3
    CHECK(xml.find("<Constant number=\"3\" />") != std::string::npos);

    // Increment: i = i + -1
    CHECK(xml.find("<Constant number=\"-1\" />") != std::string::npos);
}

TEST_CASE("Compiler XML - local variables use hashed names consistently") {
    const auto xml = compileSource(R"(
program test;
var x = 0;

on start() {
    var local = 5;
    local = local + 1;
    x = local;
})");

    // All references should use the same hashed name
    std::size_t firstHash = xml.find_first_of("_");
    REQUIRE(firstHash != std::string::npos);

    // Count occurrences of the hashed variable name
    std::string hashedName;
    // Find the pattern: variableName="NAME_" followed by hex
    std::size_t varNamePos = xml.find("variableName=\"");
    REQUIRE(varNamePos != std::string::npos);
    varNamePos += std::string("variableName=\"").size();
    std::size_t endQuote = xml.find('"', varNamePos);
    REQUIRE(endQuote != std::string::npos);
    hashedName = xml.substr(varNamePos, endQuote - varNamePos);

    // Every variableName reference should use this same name
    int count = 0;
    std::size_t pos = 0;
    while ((pos = xml.find("variableName=\"" + hashedName + "\"", pos)) !=
           std::string::npos) {
        count++;
        pos += hashedName.size() + 14; // "variableName=" + quotes + name
    }
    CHECK(count >= 3); // declaration + assignment + x = local
}

TEST_CASE("Compiler XML - nested if scope variable resolution") {
    const auto xml = compileSource(R"(
program test;

on start() {
    var a = 1;
    if (a > 0) {
        var b = 2;
        a = a + b;
    }
})");

    // Condition uses hashed name
    CHECK(xml.find("variableName=\"a") != std::string::npos);
    // All references inside if body use same hashed name
    std::size_t count = 0;
    std::string searchStr = "variableName=\"";
    std::size_t pos = 0;
    while ((pos = xml.find(searchStr, pos)) != std::string::npos) {
        count++;
        pos += searchStr.size();
    }
    CHECK(count >= 4); // a=1, a>0 condition, b=2, a=a+b (two refs)
}

TEST_CASE("Compiler XML - repeat loop") {
    const auto xml = compileSource(R"(
program test;

on start() {
    repeat 3 {
        x = x + 1;
    }
})");

    CHECK(xml.find("<Repeat id=\"") != std::string::npos);
    CHECK(xml.find("style=\"repeat\"") != std::string::npos);
    CHECK(xml.find("<Constant number=\"3\" />") != std::string::npos);
}

TEST_CASE("Compiler XML - while loop") {
    const auto xml = compileSource(R"(
program test;

on start() {
    while (x > 5) {
        x = x - 1;
    }
})");

    CHECK(xml.find("<While id=\"") != std::string::npos);
    CHECK(xml.find("style=\"while\"") != std::string::npos);
    CHECK(xml.find("<Comparison op=\"g\" style=\"op-gt\">") !=
          std::string::npos);
}

TEST_CASE("Compiler XML - if else structure") {
    const auto xml = compileSource(R"(
program test;

on start() {
    if (x > 5) {
        x = 10;
    } else {
        x = 0;
    }
})");

    CHECK(xml.find("<If id=\"") != std::string::npos);
    CHECK(xml.find("style=\"if\"") != std::string::npos);
    CHECK(xml.find("<ElseIf id=\"") != std::string::npos);
    CHECK(xml.find("style=\"else\"") != std::string::npos);
    CHECK(xml.find("<Constant style=\"true\" bool=\"true\" />") !=
          std::string::npos);
}

TEST_CASE("Compiler XML - SetVariable with expression") {
    const auto xml = compileSource(R"(
program test;
var x = 0;
var y = 0;

on start() {
    x = y + 1;
})");

    CHECK(xml.find("<BinaryOp op=\"+\" style=\"op-add\">") !=
          std::string::npos);
    CHECK(xml.find("<Constant number=\"1\" />") != std::string::npos);
}

TEST_CASE("Compiler XML - SetInput set-craft-property") {
    const auto xml = compileSource(R"(
program test;

on start() {
    perform Craft.setInput("throttle", 1);
})");

    CHECK(xml.find("<SetCraftProperty id=\"") != std::string::npos);
    CHECK(xml.find("property=\"setInput\"") != std::string::npos);
}

TEST_CASE("Compiler XML - ListIndex expression") {
    const auto xml = compileSource(R"(
program test;
list nums = [1, 2, 3];

on start() {
    var x = nums[0];
})");

    CHECK(xml.find("<ListOp op=\"get\" style=\"list-get\">") !=
          std::string::npos);
    CHECK(xml.find("list=\"true\"") != std::string::npos);
    CHECK(xml.find("variableName=\"nums\"") != std::string::npos);
    CHECK(xml.find("<Constant number=\"0\" />") != std::string::npos);
}

TEST_CASE("Compiler XML - comparison operators") {
    const auto xml = compileSource(R"(
program test;
var x = 0;

on start() {
    if (x == 5) {
        x = 1;
    }
    if (x != 3) {
        x = 2;
    }
    if (x > 1) {
        x = 3;
    }
    if (x < 10) {
        x = 4;
    }
    if (x >= 2) {
        x = 5;
    }
    if (x <= 8) {
        x = 6;
    }
})");

    CHECK(xml.find("<Comparison op=\"=\" style=\"op-eq\">") != std::string::npos);
    CHECK(xml.find("<Comparison op=\"!=\" style=\"op-ne\">") != std::string::npos);
    CHECK(xml.find("<Comparison op=\"g\" style=\"op-gt\">") != std::string::npos);
    CHECK(xml.find("<Comparison op=\"l\" style=\"op-lt\">") != std::string::npos);
    CHECK(xml.find("<Comparison op=\">=\" style=\"op-gte\">") !=
          std::string::npos);
    CHECK(xml.find("<Comparison op=\"&lt;=\" style=\"op-lte\">") !=
          std::string::npos);
}

TEST_CASE("Compiler XML - not expression") {
    const auto xml = compileSource(R"(
program test;
var flag = true;

on start() {
    if (not flag) {
        x = 1;
    }
})");

    CHECK(xml.find("<Not style=\"op-not\">") != std::string::npos);
}

TEST_CASE("Compiler XML - custom expression emission") {
    const auto xml = compileSource(R"(
program test;

expr myExpr(x) x + 1;
expr add3(a, b, c) a + b + c;
)");

    CHECK(xml.find("<CustomExpression name=\"myExpr\"") != std::string::npos);
    CHECK(xml.find("style=\"custom-expression\"") != std::string::npos);
    CHECK(xml.find("callFormat=\"myExpr (0) \"") != std::string::npos);
    CHECK(xml.find("format=\"myExpr |x|  \"") != std::string::npos);
    CHECK(xml.find("<BinaryOp op=\"+\" style=\"op-add\">") != std::string::npos);

    CHECK(xml.find("<CustomExpression name=\"add3\"") != std::string::npos);
    CHECK(xml.find("callFormat=\"add3 (0) (1) (2) \"") != std::string::npos);
    CHECK(xml.find("format=\"add3 |a| |b| |c|  \"") !=
          std::string::npos);
}

TEST_CASE("Compiler XML - emit inside on block") {
    const auto xml = compileSource(R"(
program test;

on start() {
    emit testEvent();
})");

    CHECK(xml.find("<BroadcastMessage id=\"") != std::string::npos);
    CHECK(xml.find("style=\"broadcast-msg\"") != std::string::npos);
    CHECK(xml.find("message=\"testEvent\"") != std::string::npos);
    CHECK(xml.find("global=\"false\"") != std::string::npos);
    CHECK(xml.find("local=\"true\"") != std::string::npos);
}

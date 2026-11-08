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
    do Game.display("Hello world");
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
    // Top level initializers are assigned at the top of start, before the
    // handler's own statements
    CHECK(xml.find("<SetVariable id=\"1\" style=\"set-variable\">") !=
          std::string::npos);
    CHECK(xml.find("<SetVariable id=\"2\" style=\"set-variable\">") !=
          std::string::npos);
    CHECK(xml.find("<DisplayMessage id=\"3\" style=\"display\">") !=
          std::string::npos);
    CHECK(xml.find("<Constant text=\"Hello world\" />") != std::string::npos);
    CHECK(xml.find("<Constant number=\"7\" />") != std::string::npos);
    CHECK(xml.find("</DisplayMessage>") != std::string::npos);
    CHECK(xml.find("</Instructions>") != std::string::npos);
    CHECK(xml.find("<Expressions />") != std::string::npos);
    CHECK(xml.find("</Program>") != std::string::npos);
}

TEST_CASE("Compiler XML - localLog maps to LogMessage") {
    const auto xml = compileSource(R"(
program test;
on start() {
    do Game.localLog("Hello log");
})");

    CHECK(xml.find("<LogMessage id=\"1\" style=\"log\">") !=
          std::string::npos);
    CHECK(xml.find("<Constant text=\"Hello log\" />") != std::string::npos);
    CHECK(xml.find("</LogMessage>") != std::string::npos);
    CHECK(xml.find("<DisplayMessage") == std::string::npos);
    CHECK(xml.find("<LogFlight") == std::string::npos);
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
    do Game.display(x);
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
    CHECK(xml.find("<Comparison op=\"le\" style=\"op-lte\">") !=
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
    CHECK(xml.find("<Comparison op=\"ge\" style=\"op-gte\">") !=
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

    // Count occurrences of the hashed variable name. The first variableName
    // in the XML may be the top level global (top level initializers are
    // emitted first), so pick the first hashed local name instead.
    std::string hashedName;
    std::size_t searchPos = 0;
    const std::string prefix = "variableName=\"";
    while ((searchPos = xml.find(prefix, searchPos)) != std::string::npos) {
        const std::size_t start = searchPos + prefix.size();
        const std::size_t endQuote = xml.find('"', start);
        REQUIRE(endQuote != std::string::npos);
        const std::string candidate = xml.substr(start, endQuote - start);
        if (candidate.find('_') != std::string::npos) {
            hashedName = candidate;
            break;
        }
        searchPos = endQuote + 1;
    }
    REQUIRE(!hashedName.empty());

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
    // Vizzy marks the else branch with a bare true constant
    CHECK(xml.find("<Constant bool=\"true\" />") != std::string::npos);
    CHECK(xml.find("<Constant style=\"true\" bool=\"true\" />") ==
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
    do Craft.setInput("throttle", 1);
})");

    CHECK(xml.find("<SetCraftProperty property=") != std::string::npos);
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
    CHECK(xml.find("<Constant number=\"1\" />") != std::string::npos);
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
    // Vizzy has no not-equal operator: != is emitted as Not(=)
    CHECK(xml.find("op=\"!=\"") == std::string::npos);
    CHECK(xml.find("<Not style=\"op-not\">") != std::string::npos);
    CHECK(xml.find("<Comparison op=\"g\" style=\"op-gt\">") != std::string::npos);
    CHECK(xml.find("<Comparison op=\"l\" style=\"op-lt\">") != std::string::npos);
    CHECK(xml.find("<Comparison op=\"ge\" style=\"op-gte\">") !=
          std::string::npos);
    CHECK(xml.find("<Comparison op=\"le\" style=\"op-lte\">") !=
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

    // Vizzy writes callFormat first and ends the format with the return slot
    CHECK(xml.find("<CustomExpression callFormat=\"myExpr (0) \"") !=
          std::string::npos);
    CHECK(xml.find("style=\"custom-expression\"") != std::string::npos);
    CHECK(xml.find("format=\"myExpr |x|  return (0)\"") != std::string::npos);
    CHECK(xml.find("<BinaryOp op=\"+\" style=\"op-add\">") != std::string::npos);

    CHECK(xml.find("callFormat=\"add3 (0) (1) (2) \"") != std::string::npos);
    CHECK(xml.find("format=\"add3 |a| |b| |c|  return (0)\"") !=
          std::string::npos);
}

TEST_CASE("Compiler XML - emit inside on block") {
    const auto xml = compileSource(R"(
program test;

on start() {
    emit testEvent();
})");

    // global/local lead the attribute list and the message name is a
    // child constant, never a message attribute
    CHECK(xml.find("<BroadcastMessage global=\"false\" local=\"true\" "
                   "id=\"") != std::string::npos);
    CHECK(xml.find("style=\"broadcast-msg\"") != std::string::npos);
    CHECK(xml.find("<Constant text=\"testEvent\" />") != std::string::npos);
    CHECK(xml.find("<Constant number=\"0\" />") != std::string::npos);
    CHECK(xml.find("message=\"testEvent\"") == std::string::npos);
}

TEST_CASE("Compiler XML - emit targets map to the three sample styles") {
    const auto xml = compileSource(R"(
program test;

on start() {
    emit localEvent();
    emit craftEvent() to craft;
    emit nearbyEvent() to nearby;
})");

    // Only these three style/flag combinations occur in the reference games
    CHECK(xml.find("<BroadcastMessage global=\"false\" local=\"true\" "
                   "id=\"") != std::string::npos);
    CHECK(xml.find("style=\"broadcast-msg\"") != std::string::npos);

    CHECK(xml.find("<BroadcastMessage global=\"false\" local=\"false\" "
                   "id=\"") != std::string::npos);
    CHECK(xml.find("style=\"broadcast-msg-craft\"") != std::string::npos);

    CHECK(xml.find("<BroadcastMessage global=\"true\" local=\"true\" "
                   "id=\"") != std::string::npos);
    CHECK(xml.find("style=\"broadcast-msg-all-crafts\"") != std::string::npos);

    CHECK(xml.find("broadcast-msg-nearby") == std::string::npos);
}

TEST_CASE("Compiler XML - one thread per handler with its own position") {
    // Reference programs keep every handler in its own <Instructions>
    // container, each starting in its own canvas column.
    const auto xml = compileSource(R"(
program test;

on start() {
    x = 1;
}

on update() {
    x = 2;
}

instr helper() {
    x = 3;
})");

    std::size_t threads = 0;
    std::size_t pos = 0;
    while ((pos = xml.find("<Instructions>", pos)) != std::string::npos) {
        threads++;
        pos += 13;
    }
    CHECK(threads == 3);

    CHECK(xml.find("pos=\"-10,-20\"") != std::string::npos);
    CHECK(xml.find("pos=\"390,-20\"") != std::string::npos);
    CHECK(xml.find("pos=\"790,-20\"") != std::string::npos);

    // only the head block of a thread carries a position
    CHECK(xml.find("<SetVariable id=\"1\" style=\"set-variable\" pos=") ==
          std::string::npos);
}

TEST_CASE("Compiler XML - event parameters are not written as Parameter") {
    const auto xml = compileSource(R"(
program test;

on update() {
    x = 1;
})");

    CHECK(xml.find("<Parameter") == std::string::npos);
    CHECK(xml.find("<Event event=\"ReceiveMessage\" id=\"0\" "
                   "style=\"receive-msg\"") != std::string::npos);
    // receive-msg carries its message name as a child constant
    CHECK(xml.find("<Constant canReplace=\"false\" text=\"update\" />") !=
          std::string::npos);
}

TEST_CASE("Compiler XML - list declaration keeps Items container") {
    const auto xml = compileSource(R"(
program test;
list engines = [1, 2, 3];
var fuel = 0;

on start() {
    x = 1;
})");

    CHECK(xml.find("<Variable name=\"engines\">\n      <Items />\n    "
                   "</Variable>") != std::string::npos);
    // a list never carries the number attribute a plain variable has
    CHECK(xml.find("<Variable name=\"engines\" number=") ==
          std::string::npos);
    CHECK(xml.find("<Variable name=\"fuel\" number=\"0\" />") !=
          std::string::npos);

    // Variables follow declaration order, not alphabetical order
    const auto engines = xml.find("<Variable name=\"engines\"");
    const auto fuel = xml.find("<Variable name=\"fuel\"");
    REQUIRE(engines != std::string::npos);
    REQUIRE(fuel != std::string::npos);
    CHECK(engines < fuel);
}

TEST_CASE("Compiler XML - instruction definition format") {
    const auto xml = compileSource(R"(
program test;

instr burn(fuelRate) {
    fuel = fuel - fuelRate;
}

instr noop() {
    fuel = fuel;
})");

    CHECK(xml.find("<CustomInstruction callFormat=\"burn (0) \" "
                   "format=\"burn |fuelRate| \" name=\"burn\" id=\"0\" "
                   "style=\"custom-instruction\"") != std::string::npos);
    // no parameters: the trailing gap disappears entirely
    CHECK(xml.find("callFormat=\"noop\" format=\"noop\" name=\"noop\"") !=
          std::string::npos);
    CHECK(xml.find("<CallCustomInstruction call=\"burn\"") ==
          std::string::npos);
}

TEST_CASE("Compiler XML - do of a user instruction") {
    const auto xml = compileSource(R"(
program test;

instr burn(fuelRate) {
    fuel = fuel - fuelRate;
}

on start() {
    do burn(2);
})");

    CHECK(xml.find("<CallCustomInstruction call=\"burn\" id=\"") !=
          std::string::npos);
    CHECK(xml.find("style=\"call-custom-instruction\"") != std::string::npos);
    CHECK(xml.find("<Constant number=\"2\" />") != std::string::npos);
}

TEST_CASE("Compiler XML - compound assignment keeps its operator") {
    const auto xml = compileSource(R"(
program test;
var c;
var d;

on start() {
    c += 1;
    c -= 2;
    d *= 3;
    d /= 4;
})");

    // += and -= lower to Vizzy's "change by" block, not a plain store
    CHECK(xml.find("<ChangeVariable id=\"1\" style=\"change-variable\">") !=
          std::string::npos);
    CHECK(xml.find("<ChangeVariable id=\"2\" style=\"change-variable\">") !=
          std::string::npos);
    CHECK(xml.find("<Constant number=\"-2\" />") != std::string::npos);

    // *= and /= have no dedicated block: keep the operator in a BinaryOp
    CHECK(xml.find("<BinaryOp op=\"*\" style=\"op-mul\">") !=
          std::string::npos);
    CHECK(xml.find("<BinaryOp op=\"/\" style=\"op-div\">") !=
          std::string::npos);

    // none of the four may collapse into "c = 1"
    CHECK(xml.find("<SetVariable id=\"1\"") == std::string::npos);
    CHECK(xml.find("<SetVariable id=\"2\"") == std::string::npos);
}

TEST_CASE("Compiler XML - list element assignment uses SetList") {
    const auto xml = compileSource(R"(
program test;
list nums;

on start() {
    nums[0] = 5;
    nums[1] += 2;
})");

    // list, index and new value in that order, never SetVariable on the
    // whole list
    CHECK(xml.find("<SetList op=\"set\" id=\"1\" style=\"list-set\">") !=
          std::string::npos);
    CHECK(xml.find("<SetList op=\"set\" id=\"2\" style=\"list-set\">") !=
          std::string::npos);
    CHECK(xml.find("<Variable list=\"true\" local=\"false\" "
                   "variableName=\"nums\" />") != std::string::npos);

    // the compound form reads the old element first
    CHECK(xml.find("<ListOp op=\"get\" style=\"list-get\">") !=
          std::string::npos);
    CHECK(xml.find("<BinaryOp op=\"+\" style=\"op-add\">") !=
          std::string::npos);
    CHECK(xml.find("<SetVariable") == std::string::npos);
}

TEST_CASE("Compiler XML - top level initializers without a start handler "
          "get their own FlightStart thread") {
    const auto xml = compileSource(R"(
program test;
var x = 5;
list nums = [1, 2];
)");

    // No on start: the declarations get a FlightStart thread of their own,
    // with the initializers assigned at the top of it
    CHECK(xml.find("<Event event=\"FlightStart\" id=\"0\" style=\"flight-start\" "
                   "pos=\"-10,-20\" />") != std::string::npos);
    CHECK(xml.find("<SetVariable id=\"1\" style=\"set-variable\">") !=
          std::string::npos);
    CHECK(xml.find("<Constant number=\"5\" />") != std::string::npos);
    CHECK(xml.find("<SetVariable id=\"2\" style=\"list-init\">") !=
          std::string::npos);
    CHECK(xml.find("<ListOp op=\"create\" style=\"list-create\">") !=
          std::string::npos);
    CHECK(xml.find("<Constant text=\"1,2\" />") != std::string::npos);
}

TEST_CASE("Compiler XML - emit to script parses and broadcasts") {
    const auto xml = compileSource(R"(
program test;
on start() {
    emit testEvent() to script;
}
)");

    // `to script` is accepted and uses the same broadcast-msg shape as a
    // plain emit: message name as a child constant, empty data list
    CHECK(xml.find("<BroadcastMessage global=\"false\" local=\"true\" "
                   "id=\"1\" style=\"broadcast-msg\">") !=
          std::string::npos);
    CHECK(xml.find("<Constant text=\"testEvent\" />") != std::string::npos);
    CHECK(xml.find("<Constant number=\"0\" />") != std::string::npos);
}

TEST_CASE("Compiler XML - custom event params resolve to data positions") {
    const auto xml = compileSource(R"(
program test;
var x = 0;
var y = 0;

on customEvent(a, b) {
    x = a;
    y = b;
}
)");

    // Custom events deliver their parameters through Vizzy's built-in data
    // list, indexed by 1 based position, referenced as local data
    CHECK(xml.find("<Event event=\"ReceiveMessage\" id=\"0\" "
                   "style=\"receive-msg\"") != std::string::npos);
    CHECK(xml.find("<Constant canReplace=\"false\" text=\"customEvent\" />") !=
          std::string::npos);
    CHECK(xml.find("<ListOp op=\"get\" style=\"list-get\">") !=
          std::string::npos);
    CHECK(xml.find("variableName=\"data\"") != std::string::npos);
    CHECK(xml.find("<Constant number=\"1\" />") != std::string::npos);
    CHECK(xml.find("<Constant number=\"2\" />") != std::string::npos);
    CHECK(xml.find("local=\"true\"") != std::string::npos);
}

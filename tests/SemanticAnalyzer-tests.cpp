#include "doctest.h"
#include <string>
#include <vector>

import junopl.lexer;
import junopl.parser;
import junopl.parser.nodes;
import junopl.semantic;
import junopl.semantic.diagnostic;
import junopl.tests.filehandler;

namespace {
std::vector<JunoPL::Diagnostic> analyze(const std::string &source) {
    JunoPL::Tests::FileHandlerTest fileHandler;
    fileHandler.setSourceCode(source);

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileHandler);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);
    REQUIRE(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    JunoPL::SemanticAnalyzer analyzer;
    return analyzer.analyze(*parseOutput.root);
}
} // namespace

TEST_CASE("Semantic - valid standard library calls") {
    // Instructions with and without the perform keyword
    CHECK(
        analyze("program test; on start() { perform Game.delay(1); }").empty());
    CHECK(analyze("program test; on start() { Game.delay(1); }").empty());
    CHECK(analyze("program test; on start() { Game.flightLog(\"hi\", true); }")
              .empty());
    CHECK(
        analyze("program test; on start() { Craft.lockPrograde(); }").empty());

    // Instruction vs expression forms of the same name
    CHECK(analyze("program test; on start() { Input.throttle(0.5); }").empty());
    CHECK(analyze("program test; on start() { var t = Input.throttle(); }")
              .empty());
    CHECK(analyze("program test; on start() { var t = Input.slider(1); }")
              .empty());

    // Expressions
    CHECK(
        analyze("program test; on start() { var x = Math.abs(-1); }").empty());
    CHECK(analyze("program test; on start() { var x = Math.atan2(1, 2); }")
              .empty());
    CHECK(analyze("program test; on start() { var s = String.join(\"a\", "
                  "\"b\", \"c\"); }")
              .empty());
    CHECK(analyze("program test; on start() { var v = Vector.vec(1, 2, 3); }")
              .empty());
    CHECK(
        analyze("program test; on start() { var n = Vector.vecX(); }").empty());
    CHECK(analyze("program test; on start() { var a = Altitude.getAGL(); }")
              .empty());
    CHECK(
        analyze("program test; on start() { var f = Game.frequencyOf(4, 4); }")
            .empty());
    CHECK(analyze(
              "program test; on start() { var p = Planet.radius(\"Earth\"); }")
              .empty());

    // Overloaded arities
    CHECK(analyze("program test; on start() { var v = Vector.vec(1, 2, 3); "
                  "Camera.targetOffset(v); }")
              .empty());
    CHECK(analyze("program test; on start() { Camera.targetOffset(1, 2, 3); }")
              .empty());
    CHECK(
        analyze("program test; on start() { var n = Craft.name(); }").empty());
    CHECK(analyze("program test; on start() { var n = Craft.name(\"other\"); }")
              .empty());

    // Qualified list operations
    CHECK(analyze("program test; on start() { list items = [1]; "
                  "Lists.insert(items, 3); }")
              .empty());
    CHECK(analyze("program test; on start() { list items = [1]; "
                  "Lists.insert(items, 3, 4, 5); }")
              .empty());
    CHECK(analyze("program test; on start() { list items = [1]; "
                  "var n = Lists.length(items); }")
              .empty());
}

TEST_CASE("Semantic - arity diagnostics") {
    auto tooMany = analyze("program test; on start() { Game.delay(1, 2); }");
    REQUIRE(tooMany.size() == 1);
    CHECK(tooMany[0].message == "'Game.delay' expects 1 argument, got 2");

    auto tooFew = analyze("program test; on start() { Game.beep(440, 1); }");
    REQUIRE(tooFew.size() == 1);
    CHECK(tooFew[0].message == "'Game.beep' expects 3 arguments, got 2");

    auto insert = analyze("program test; on start() { list items = [1]; "
                          "Lists.insert(items); }");
    REQUIRE(insert.size() == 1);
    CHECK(insert[0].message ==
          "'Lists.insert' expects at least 2 arguments, got 1");

    auto join =
        analyze("program test; on start() { var s = String.join(\"a\"); }");
    REQUIRE(join.size() == 1);
    CHECK(join[0].message ==
          "'String.join' expects at least 2 arguments, got 1");

    auto targetOffset =
        analyze("program test; on start() { Camera.targetOffset(1, 2); }");
    REQUIRE(targetOffset.size() == 1);
    CHECK(targetOffset[0].message ==
          "'Camera.targetOffset' expects 1 argument or 3 arguments, got 2");
}

TEST_CASE("Semantic - unknown and misused calls") {
    auto unknown = analyze("program test; on start() { Game.noSuch(1); }");
    REQUIRE(unknown.size() == 1);
    CHECK(unknown[0].message ==
          "'Game.noSuch' is not defined in the standard library");

    // Receiver-style list calls (items.insert) are not part of the language
    auto receiver = analyze("program test; on start() { list items = [1]; "
                            "items.insert(3); }");
    REQUIRE(receiver.size() == 1);
    CHECK(receiver[0].message ==
          "'items.insert' is not defined in the standard library");

    // An instruction used as an expression is rejected
    auto wrongOp =
        analyze("program test; on start() { var x = Game.delay(1); }");
    REQUIRE(wrongOp.size() == 1);
    CHECK(wrongOp[0].message ==
          "'Game.delay' is not defined in the standard library");
}

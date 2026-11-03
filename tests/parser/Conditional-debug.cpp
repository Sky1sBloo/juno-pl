#include "doctest.h"
#include <string>
#include <variant>
import junopl.lexer;
import junopl.lexer.tokens;
import junopl.parser;
import junopl.parser.nodes;
import junopl.parser.nodes.statements;
import junopl.parser.nodes.expressions;
import junopl.tests.filehandler;

TEST_CASE("Simple conditional") {
    JunoPL::Tests::FileHandlerTest fileHandler;
    fileHandler.setSourceCode("program test; on start() { if (true) { } }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileHandler);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    for (const auto& e : parseOutput.errors) {
        INFO("Error: ", e.message());
    }
    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);
    REQUIRE(parseOutput.root->body.size() == 1);
}

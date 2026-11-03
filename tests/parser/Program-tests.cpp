#include "doctest.h"
#include <variant>

import junopl.lexer;
import junopl.lexer.tokens;
import junopl.parser;
import junopl.parser.nodes;
import junopl.tests.filehandler;

TEST_CASE("Parser Program - program name only") {
    JunoPL::Tests::FileHandlerTest fileHandler;
    fileHandler.setSourceCode("program myApp;");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileHandler);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);
    CHECK(parseOutput.root->programName == "myApp");
    CHECK(parseOutput.root->body.empty());
}

TEST_CASE("Parser Program - with imports") {
    JunoPL::Tests::FileHandlerTest fileHandler;
    fileHandler.setSourceCode("program myApp; import \"utils.js\"; import \"math.js\";");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileHandler);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);
    CHECK(parseOutput.root->programName == "myApp");
    REQUIRE(parseOutput.root->body.size() == 2);
    CHECK(std::holds_alternative<JunoPL::ImportNode>(parseOutput.root->body.at(0)));
    CHECK(std::holds_alternative<JunoPL::ImportNode>(parseOutput.root->body.at(1)));
    CHECK(std::get<JunoPL::ImportNode>(parseOutput.root->body.at(0)).path == "utils.js");
    CHECK(std::get<JunoPL::ImportNode>(parseOutput.root->body.at(1)).path == "math.js");
}

TEST_CASE("Parser Program - missing program keyword") {
    JunoPL::Tests::FileHandlerTest fileHandler;
    fileHandler.setSourceCode("myApp;");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileHandler);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(!parseOutput.errors.empty());
}

TEST_CASE("Parser Program - missing semicolon after program name") {
    JunoPL::Tests::FileHandlerTest fileHandler;
    fileHandler.setSourceCode("program myApp import \"utils.js\";");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileHandler);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(!parseOutput.errors.empty());
}

TEST_CASE("Parser Program - empty input") {
    JunoPL::Tests::FileHandlerTest fileHandler;
    fileHandler.setSourceCode("");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileHandler);

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(!parseOutput.errors.empty());
}

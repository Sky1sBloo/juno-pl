#include "doctest.h"
#include <variant>

import junopl.lexer;
import junopl.lexer.tokens;
import junopl.parser;
import junopl.parser.nodes;
import junopl.parser.nodes.statements;
import junopl.tests.filereader;

TEST_CASE("Parser Function - basic") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; instr doNothing() { }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);
    REQUIRE(parseOutput.root->body.size() == 1);
    REQUIRE(std::holds_alternative<JunoPL::FunctionNode>(parseOutput.root->body.at(0)));

    const auto &func = std::get<JunoPL::FunctionNode>(parseOutput.root->body.at(0));
    CHECK(func.identifier == "doNothing");
    CHECK(func.params.empty());
    CHECK(func.body.body.empty());
}

TEST_CASE("Parser Function - with params") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; instr add(a, b) { }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);
    REQUIRE(parseOutput.root->body.size() == 1);

    const auto &func = std::get<JunoPL::FunctionNode>(parseOutput.root->body.at(0));
    CHECK(func.identifier == "add");
    REQUIRE(func.params.size() == 2);
    CHECK(func.params[0] == "a");
    CHECK(func.params[1] == "b");
}

TEST_CASE("Parser Function - with body statements") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; instr main() { var x = 1; var y = 2; }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);
    REQUIRE(parseOutput.root->body.size() == 1);

    const auto &func = std::get<JunoPL::FunctionNode>(parseOutput.root->body.at(0));
    CHECK(func.identifier == "main");
    REQUIRE(func.body.body.size() == 2);
}

TEST_CASE("Parser Function - missing identifier") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; instr () { }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(!parseOutput.errors.empty());
}

TEST_CASE("Parser Function - missing body") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; instr main()");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(!parseOutput.errors.empty());
}

TEST_CASE("Parser Multiple functions") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; instr a() { } instr b() { } instr c() { }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);
    REQUIRE(parseOutput.root->body.size() == 3);
    CHECK(std::holds_alternative<JunoPL::FunctionNode>(parseOutput.root->body.at(0)));
    CHECK(std::holds_alternative<JunoPL::FunctionNode>(parseOutput.root->body.at(1)));
    CHECK(std::holds_alternative<JunoPL::FunctionNode>(parseOutput.root->body.at(2)));
}

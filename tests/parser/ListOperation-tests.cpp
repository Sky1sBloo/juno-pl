#include "doctest.h"
#include <variant>

import junopl.lexer;
import junopl.lexer.tokens;
import junopl.parser;
import junopl.parser.nodes;
import junopl.parser.nodes.statements;
import junopl.parser.nodes.expressions;
import junopl.tests.filereader;

TEST_CASE("Parser List Operation - insert") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode(
        "program test; on start() { list items = [1, 2]; Lists.insert(items, 3); }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    REQUIRE(event.body.body.size() == 2);
    REQUIRE(std::holds_alternative<JunoPL::PerformInstruction>(event.body.body.at(1)->statement));

    const auto &instr = std::get<JunoPL::PerformInstruction>(event.body.body.at(1)->statement);
    CHECK(instr.qualifier == "Lists");
    CHECK(instr.identifier == "insert");
    REQUIRE(instr.params.size() == 2);
    CHECK(std::get<JunoPL::Value>(instr.params[0]->value).value == "items");
    CHECK(std::get<JunoPL::Value>(instr.params[1]->value).value == "3");
}

TEST_CASE("Parser List Operation - remove") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode(
        "program test; on start() { list items = [1, 2]; Lists.remove(items, 0); }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    const auto &instr = std::get<JunoPL::PerformInstruction>(event.body.body.at(1)->statement);
    CHECK(instr.qualifier == "Lists");
    CHECK(instr.identifier == "remove");
    CHECK(instr.params.size() == 2);
}

TEST_CASE("Parser List Operation - clear") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode(
        "program test; on start() { list items = [1]; Lists.clear(items); }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    const auto &instr = std::get<JunoPL::PerformInstruction>(event.body.body.at(1)->statement);
    CHECK(instr.identifier == "clear");
    REQUIRE(instr.params.size() == 1);
    CHECK(std::get<JunoPL::Value>(instr.params[0]->value).value == "items");
}

TEST_CASE("Parser List Operation - sort") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode(
        "program test; on start() { list items = [3, 1, 2]; Lists.sort(items); }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    const auto &instr = std::get<JunoPL::PerformInstruction>(event.body.body.at(1)->statement);
    CHECK(instr.identifier == "sort");
    CHECK(instr.params.size() == 1);
}

TEST_CASE("Parser List Operation - reverse") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode(
        "program test; on start() { list items = [1, 2]; Lists.reverse(items); }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    const auto &instr = std::get<JunoPL::PerformInstruction>(event.body.body.at(1)->statement);
    CHECK(instr.identifier == "reverse");
    CHECK(instr.params.size() == 1);
}

TEST_CASE("Parser List Index - expression") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { list items = [10, 20, 30]; perform show(items[1]); }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    const auto &perform = std::get<JunoPL::PerformInstruction>(event.body.body.at(1)->statement);
    REQUIRE(perform.params.size() == 1);
    REQUIRE(std::holds_alternative<JunoPL::ListIndex>(perform.params[0]->value));

    const auto &listIdx = std::get<JunoPL::ListIndex>(perform.params[0]->value);
    CHECK(listIdx.identifier == "items");
    CHECK(std::get<JunoPL::Value>(listIdx.index->value).value == "1");
}

TEST_CASE("Parser List Declaration - empty") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { list items = []; }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    const auto &listDecl = std::get<JunoPL::ListDeclaration>(event.body.body.at(0)->statement);
    CHECK(listDecl.identifier == "items");
    CHECK(listDecl.values.empty());
}

TEST_CASE("Parser List Declaration - no value") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { list items; }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    const auto &listDecl = std::get<JunoPL::ListDeclaration>(event.body.body.at(0)->statement);
    CHECK(listDecl.identifier == "items");
    CHECK(listDecl.values.empty());
}

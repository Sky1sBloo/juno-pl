#include "doctest.h"
#include <variant>

import junopl.lexer;
import junopl.lexer.tokens;
import junopl.parser.handlers;
import junopl.parser.nodes.expressions;
import junopl.tests.filereader;

TEST_CASE("Parser Perform Instruction - no params") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("perform doSomething();");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::TokenList tokens;
    for (std::size_t i = 0; i < lexOutput.tokens.size(); ++i) {
        tokens.push_back(lexOutput.tokens.at(i));
    }

    auto result = JunoPL::parsePerformInstruction(tokens);
    REQUIRE(result.has_value());
    CHECK(result.value().identifier == "doSomething");
    CHECK(result.value().params.empty());
}

TEST_CASE("Parser Perform Instruction - with params") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("perform movePlayer(1, \"right\", true);");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::TokenList tokens;
    for (std::size_t i = 0; i < lexOutput.tokens.size(); ++i) {
        tokens.push_back(lexOutput.tokens.at(i));
    }

    auto result = JunoPL::parsePerformInstruction(tokens);
    REQUIRE(result.has_value());
    CHECK(result.value().identifier == "movePlayer");
    REQUIRE(result.value().params.size() == 3);
    CHECK(std::get<JunoPL::Value>(result.value().params[0]->value).value ==
          "1");
    CHECK(std::get<JunoPL::Value>(result.value().params[1]->value).value ==
          "right");
    CHECK(std::get<JunoPL::Value>(result.value().params[2]->value).value ==
          "true");
}

TEST_CASE("Parser Perform Instruction - expression params") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("perform calc(x + 1, y * 2);");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::TokenList tokens;
    for (std::size_t i = 0; i < lexOutput.tokens.size(); ++i) {
        tokens.push_back(lexOutput.tokens.at(i));
    }

    auto result = JunoPL::parsePerformInstruction(tokens);
    REQUIRE(result.has_value());
    CHECK(result.value().identifier == "calc");
    REQUIRE(result.value().params.size() == 2);
    CHECK(std::holds_alternative<JunoPL::BinaryOp>(
        result.value().params[0]->value));
    CHECK(std::holds_alternative<JunoPL::BinaryOp>(
        result.value().params[1]->value));
}

TEST_CASE("Parser Perform Instruction - missing perform keyword") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("doSomething()");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::TokenList tokens;
    for (std::size_t i = 0; i < lexOutput.tokens.size(); ++i) {
        tokens.push_back(lexOutput.tokens.at(i));
    }

    auto result = JunoPL::parsePerformInstruction(tokens);
    CHECK(!result.has_value());
}

TEST_CASE("Parser Perform Instruction - missing identifier") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("perform ()");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::TokenList tokens;
    for (std::size_t i = 0; i < lexOutput.tokens.size(); ++i) {
        tokens.push_back(lexOutput.tokens.at(i));
    }

    auto result = JunoPL::parsePerformInstruction(tokens);
    CHECK(!result.has_value());
}

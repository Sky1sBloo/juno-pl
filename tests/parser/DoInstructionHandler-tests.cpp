#include "doctest.h"
#include <string>
#include <variant>

import junopl.lexer;
import junopl.lexer.tokens;
import junopl.parser.handlers;
import junopl.parser.nodes.expressions;
import junopl.tests.filehandler;

TEST_CASE("Parser Do Instruction - no params") {
    JunoPL::Tests::FileHandlerTest fileHandler;
    fileHandler.setSourceCode("do doSomething();");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileHandler);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::TokenList tokens;
    for (std::size_t i = 0; i < lexOutput.tokens.size(); ++i) {
        tokens.push_back(lexOutput.tokens.at(i));
    }

    auto result = JunoPL::parseDoInstruction(tokens);
    REQUIRE(result.has_value());
    CHECK(result.value().identifier == "doSomething");
    CHECK(result.value().params.empty());
}

TEST_CASE("Parser Do Instruction - with params") {
    JunoPL::Tests::FileHandlerTest fileHandler;
    fileHandler.setSourceCode("do movePlayer(1, \"right\", true);");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileHandler);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::TokenList tokens;
    for (std::size_t i = 0; i < lexOutput.tokens.size(); ++i) {
        tokens.push_back(lexOutput.tokens.at(i));
    }

    auto result = JunoPL::parseDoInstruction(tokens);
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

TEST_CASE("Parser Do Instruction - expression params") {
    JunoPL::Tests::FileHandlerTest fileHandler;
    fileHandler.setSourceCode("do calc(x + 1, y * 2);");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileHandler);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::TokenList tokens;
    for (std::size_t i = 0; i < lexOutput.tokens.size(); ++i) {
        tokens.push_back(lexOutput.tokens.at(i));
    }

    auto result = JunoPL::parseDoInstruction(tokens);
    REQUIRE(result.has_value());
    CHECK(result.value().identifier == "calc");
    REQUIRE(result.value().params.size() == 2);
    CHECK(std::holds_alternative<JunoPL::BinaryOp>(
        result.value().params[0]->value));
    CHECK(std::holds_alternative<JunoPL::BinaryOp>(
        result.value().params[1]->value));
}

TEST_CASE("Parser Do Instruction - missing do keyword") {
    JunoPL::Tests::FileHandlerTest fileHandler;
    fileHandler.setSourceCode("doSomething()");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileHandler);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::TokenList tokens;
    for (std::size_t i = 0; i < lexOutput.tokens.size(); ++i) {
        tokens.push_back(lexOutput.tokens.at(i));
    }

    auto result = JunoPL::parseDoInstruction(tokens);
    CHECK(!result.has_value());
}

TEST_CASE("Parser Do Instruction - missing identifier") {
    JunoPL::Tests::FileHandlerTest fileHandler;
    fileHandler.setSourceCode("do ()");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileHandler);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::TokenList tokens;
    for (std::size_t i = 0; i < lexOutput.tokens.size(); ++i) {
        tokens.push_back(lexOutput.tokens.at(i));
    }

    auto result = JunoPL::parseDoInstruction(tokens);
    CHECK(!result.has_value());
}

TEST_CASE("Parser Do Instruction - inline keyword") {
    JunoPL::Tests::FileHandlerTest fileHandler;
    fileHandler.setSourceCode("do inline await(alt < 5000);");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileHandler);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::TokenList tokens;
    for (std::size_t i = 0; i < lexOutput.tokens.size(); ++i) {
        tokens.push_back(lexOutput.tokens.at(i));
    }

    auto result = JunoPL::parseDoInstruction(tokens);
    REQUIRE(result.has_value());
    CHECK(result.value().identifier == "await");
    CHECK(result.value().qualifier.empty());
    CHECK(result.value().isInline);
    REQUIRE(result.value().params.size() == 1);
    CHECK(std::holds_alternative<JunoPL::BinaryOp>(
        result.value().params[0]->value));
    const auto &binOp = std::get<JunoPL::BinaryOp>(result.value().params[0]->value);
    CHECK(binOp.operation == JunoPL::BinaryOp::Operation::LESS);
    CHECK(std::holds_alternative<JunoPL::Value>(binOp.lhs->value));
    CHECK(std::get<JunoPL::Value>(binOp.lhs->value).value == "alt");
    CHECK(std::holds_alternative<JunoPL::Value>(binOp.rhs->value));
    CHECK(std::get<JunoPL::Value>(binOp.rhs->value).value == "5000");
}

#include "doctest.h"
#include <variant>

import junopl.lexer;
import junopl.lexer.tokens;
import junopl.parser;
import junopl.parser.nodes;
import junopl.parser.nodes.statements;
import junopl.parser.nodes.expressions;
import junopl.tests.filereader;

TEST_CASE("Parser Var Assignment - simple assign") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { var x = 1; x = 5; }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    REQUIRE(event.body.body.size() == 2);
    REQUIRE(std::holds_alternative<JunoPL::VarAssignment>(event.body.body.at(1)->statement));

    const auto &assign = std::get<JunoPL::VarAssignment>(event.body.body.at(1)->statement);
    CHECK(assign.identifier == "x");
    CHECK(assign.op == JunoPL::VarAssignment::ASSIGN);
    CHECK(std::get<JunoPL::Value>(assign.value->value).value == "5");
}

TEST_CASE("Parser Var Assignment - plus assign") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { var x = 1; x += 3; }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    const auto &assign = std::get<JunoPL::VarAssignment>(event.body.body.at(1)->statement);
    CHECK(assign.op == JunoPL::VarAssignment::PLUS_ASSIGN);
}

TEST_CASE("Parser Var Assignment - minus assign") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { var x = 1; x -= 2; }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    const auto &assign = std::get<JunoPL::VarAssignment>(event.body.body.at(1)->statement);
    CHECK(assign.op == JunoPL::VarAssignment::MINUS_ASSIGN);
}

TEST_CASE("Parser Var Assignment - mult assign") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { var x = 1; x *= 2; }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    const auto &assign = std::get<JunoPL::VarAssignment>(event.body.body.at(1)->statement);
    CHECK(assign.op == JunoPL::VarAssignment::MULT_ASSIGN);
}

TEST_CASE("Parser Var Assignment - divide assign") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { var x = 1; x /= 2; }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    const auto &assign = std::get<JunoPL::VarAssignment>(event.body.body.at(1)->statement);
    CHECK(assign.op == JunoPL::VarAssignment::DIVIDE_ASSIGN);
}

TEST_CASE("Parser Var Assignment - with expression") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { var x = 1; x = 2 + 3 * 4; }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    const auto &assign = std::get<JunoPL::VarAssignment>(event.body.body.at(1)->statement);
    CHECK(assign.identifier == "x");
    CHECK(std::holds_alternative<JunoPL::BinaryOp>(assign.value->value));
}

TEST_CASE("Parser Var Assignment - list index assign") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { list items = [1, 2, 3]; items[0] = 10; }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    REQUIRE(event.body.body.size() == 2);
    const auto &assign = std::get<JunoPL::VarAssignment>(event.body.body.at(1)->statement);
    CHECK(assign.identifier == "items");
    REQUIRE(assign.indexExpr);
    CHECK(std::get<JunoPL::Value>(assign.indexExpr->value).value == "0");
    CHECK(assign.op == JunoPL::VarAssignment::ASSIGN);
    CHECK(std::get<JunoPL::Value>(assign.value->value).value == "10");
}

TEST_CASE("Parser Var Assignment - missing semicolon") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { var x = 1; x = 5 }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(!parseOutput.errors.empty());
}

#include "doctest.h"
#include <variant>

import junopl.lexer;
import junopl.lexer.tokens;
import junopl.parser;
import junopl.parser.nodes;
import junopl.parser.nodes.statements;
import junopl.parser.nodes.expressions;
import junopl.tests.filereader;

TEST_CASE("Parser Repeat Loop - basic") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { repeat 3 { } }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    REQUIRE(std::holds_alternative<JunoPL::RepeatLoop>(event.body.body.at(0)->statement));

    const auto &loop = std::get<JunoPL::RepeatLoop>(event.body.body.at(0)->statement);
    CHECK(std::get<JunoPL::Value>(loop.repeatNumber->value).value == "3");
    CHECK(loop.body.body.empty());
}

TEST_CASE("Parser Repeat Loop - with body") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { repeat 5 { var x = 1; } }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    const auto &loop = std::get<JunoPL::RepeatLoop>(event.body.body.at(0)->statement);
    REQUIRE(loop.body.body.size() == 1);
}

TEST_CASE("Parser While Loop - basic") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { while true { } }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    REQUIRE(std::holds_alternative<JunoPL::WhileLoop>(event.body.body.at(0)->statement));

    const auto &loop = std::get<JunoPL::WhileLoop>(event.body.body.at(0)->statement);
    CHECK(std::holds_alternative<JunoPL::Value>(loop.condition->value));
    CHECK(loop.body.body.empty());
}

TEST_CASE("Parser While Loop - with condition") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { var x = 0; while (x < 10) { x += 1; } }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    const auto &loop = std::get<JunoPL::WhileLoop>(event.body.body.at(1)->statement);
    CHECK(std::holds_alternative<JunoPL::BinaryOp>(loop.condition->value));
    REQUIRE(loop.body.body.size() == 1);
}

TEST_CASE("Parser For Loop - basic") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { for i from 0 to 10 by 1 { } }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    REQUIRE(std::holds_alternative<JunoPL::ForLoop>(event.body.body.at(0)->statement));

    const auto &loop = std::get<JunoPL::ForLoop>(event.body.body.at(0)->statement);
    CHECK(loop.iterator == "i");
    CHECK(std::get<JunoPL::Value>(loop.start->value).value == "0");
    CHECK(std::get<JunoPL::Value>(loop.end->value).value == "10");
    CHECK(std::get<JunoPL::Value>(loop.by->value).value == "1");
    CHECK(loop.body.body.empty());
}

TEST_CASE("Parser For Loop - with body") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { for i from 0 to 5 by 1 { var x = i; } }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    const auto &loop = std::get<JunoPL::ForLoop>(event.body.body.at(0)->statement);
    REQUIRE(loop.body.body.size() == 1);
}

TEST_CASE("Parser For Loop - with expression bounds") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { var n = 10; for i from 0 to n by 2 { } }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    const auto &loop = std::get<JunoPL::ForLoop>(event.body.body.at(1)->statement);
    CHECK(std::holds_alternative<JunoPL::Value>(loop.start->value));
    CHECK(std::holds_alternative<JunoPL::Value>(loop.end->value));
    CHECK(std::get<JunoPL::Value>(loop.by->value).value == "2");
}

TEST_CASE("Parser Break Statement") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { while true { break; } }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    const auto &loop = std::get<JunoPL::WhileLoop>(event.body.body.at(0)->statement);
    REQUIRE(loop.body.body.size() == 1);
    CHECK(std::holds_alternative<JunoPL::BreakStatement>(loop.body.body.at(0)->statement));
}

#include "doctest.h"
#include <variant>

import junopl.lexer;
import junopl.lexer.tokens;
import junopl.parser;
import junopl.parser.nodes;
import junopl.parser.nodes.statements;
import junopl.parser.nodes.expressions;
import junopl.tests.filereader;

TEST_CASE("Parser Conditional - if only") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { var x = 1; if (x == 1) { } }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    REQUIRE(event.body.body.size() == 2);
    REQUIRE(std::holds_alternative<JunoPL::ConditionalStatement>(event.body.body.at(1)->statement));

    const auto &cond = std::get<JunoPL::ConditionalStatement>(event.body.body.at(1)->statement);
    REQUIRE(std::holds_alternative<JunoPL::BinaryOp>(cond.ifStatement.condition->value));
    CHECK(cond.elifStatements.empty());
    CHECK(cond.elseStatement.body.empty());
}

TEST_CASE("Parser Conditional - if/else") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { if (true) { } else { var z = 1; } }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    const auto &cond = std::get<JunoPL::ConditionalStatement>(event.body.body.at(0)->statement);
    CHECK(cond.elifStatements.empty());
    CHECK(!cond.elseStatement.body.empty());
}

TEST_CASE("Parser Conditional - if/elif") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { if (true) { } elif (false) { } }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    const auto &cond = std::get<JunoPL::ConditionalStatement>(event.body.body.at(0)->statement);
    REQUIRE(cond.elifStatements.size() == 1);
    CHECK(cond.elseStatement.body.empty());
}

TEST_CASE("Parser Conditional - if/elif/else") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { if (true) { } elif (false) { } else { var z = 1; } }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    const auto &cond = std::get<JunoPL::ConditionalStatement>(event.body.body.at(0)->statement);
    REQUIRE(cond.elifStatements.size() == 1);
    CHECK(!cond.elseStatement.body.empty());
}

TEST_CASE("Parser Conditional - multiple elif") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { if (a) { } elif (b) { } elif (c) { } else { var z = 1; } }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    const auto &cond = std::get<JunoPL::ConditionalStatement>(event.body.body.at(0)->statement);
    REQUIRE(cond.elifStatements.size() == 2);
    CHECK(!cond.elseStatement.body.empty());
}

TEST_CASE("Parser Conditional - with body content") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { if (x > 0) { var y = 1; } }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    const auto &cond = std::get<JunoPL::ConditionalStatement>(event.body.body.at(0)->statement);
    REQUIRE(cond.ifStatement.body.body.size() == 1);
    CHECK(std::holds_alternative<JunoPL::VarDeclaration>(cond.ifStatement.body.body.at(0)->statement));
}

TEST_CASE("Parser Ternary - with else keyword") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; expr max(a, b) a > b ? a else b;");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &expr = std::get<JunoPL::CustomExpression>(parseOutput.root->body.at(0));
    REQUIRE(std::holds_alternative<JunoPL::TernaryOp>(expr.expression->value));

    const auto &ternary = std::get<JunoPL::TernaryOp>(expr.expression->value);
    CHECK(std::holds_alternative<JunoPL::BinaryOp>(ternary.condition->value));
    CHECK(std::holds_alternative<JunoPL::Value>(ternary.valueIfTrue->value));
    CHECK(std::holds_alternative<JunoPL::Value>(ternary.valueIfFalse->value));
}

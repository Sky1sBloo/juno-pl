#include "doctest.h"
#include <string>
#include <variant>

import junopl.lexer;
import junopl.lexer.tokens;
import junopl.parser;
import junopl.parser.nodes;
import junopl.parser.nodes.expressions;
import junopl.parser.nodes.statements;
import junopl.tests.filereader;

TEST_CASE("Parser Call Expression - basic") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; expr getVal() myFunc();");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &expr = std::get<JunoPL::CustomExpression>(parseOutput.root->body.at(0));
    REQUIRE(std::holds_alternative<JunoPL::CallExpression>(expr.expression->value));

    const auto &call = std::get<JunoPL::CallExpression>(expr.expression->value);
    CHECK(call.identifier == "myFunc");
    CHECK(call.qualifier.empty());
    CHECK(call.params.empty());
}

TEST_CASE("Parser Call Expression - with params") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; expr getVal() add(1, 2);");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &expr = std::get<JunoPL::CustomExpression>(parseOutput.root->body.at(0));
    const auto &call = std::get<JunoPL::CallExpression>(expr.expression->value);
    CHECK(call.identifier == "add");
    REQUIRE(call.params.size() == 2);
}

TEST_CASE("Parser Call Expression - qualified") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; expr getVal() math.sqrt(4);");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &expr = std::get<JunoPL::CustomExpression>(parseOutput.root->body.at(0));
    const auto &call = std::get<JunoPL::CallExpression>(expr.expression->value);
    CHECK(call.qualifier == "math");
    CHECK(call.identifier == "sqrt");
    REQUIRE(call.params.size() == 1);
}

TEST_CASE("Parser Logical Expression - and") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; expr check() a > 0 and b < 10;");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &expr = std::get<JunoPL::CustomExpression>(parseOutput.root->body.at(0));
    REQUIRE(std::holds_alternative<JunoPL::BinaryOp>(expr.expression->value));

    const auto &binop = std::get<JunoPL::BinaryOp>(expr.expression->value);
    CHECK(binop.operation == JunoPL::BinaryOp::Operation::AND);
}

TEST_CASE("Parser Logical Expression - or") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; expr check() a > 0 or b < 10;");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &expr = std::get<JunoPL::CustomExpression>(parseOutput.root->body.at(0));
    const auto &binop = std::get<JunoPL::BinaryOp>(expr.expression->value);
    CHECK(binop.operation == JunoPL::BinaryOp::Operation::OR);
}

TEST_CASE("Parser Comparison Expression - all operators") {
    auto testOp = [](const std::string &op, JunoPL::BinaryOp::Operation expected) {
        JunoPL::Tests::FileReaderTest fileReader;
        fileReader.setSourceCode("program test; expr check() a " + op + " b;");

        JunoPL::Lexer lexer;
        auto lexOutput = lexer.tokenize(fileReader);
        REQUIRE(lexOutput.errors.empty());

        JunoPL::Parser parser;
        auto parseOutput = parser.parse(lexOutput.tokens);

        CHECK(parseOutput.errors.empty());
        const auto &expr = std::get<JunoPL::CustomExpression>(parseOutput.root->body.at(0));
        const auto &binop = std::get<JunoPL::BinaryOp>(expr.expression->value);
        CHECK(binop.operation == expected);
    };

    testOp("==", JunoPL::BinaryOp::Operation::EQUAL);
    testOp("!=", JunoPL::BinaryOp::Operation::NOT_EQUAL);
    testOp(">", JunoPL::BinaryOp::Operation::GREATER);
    testOp("<", JunoPL::BinaryOp::Operation::LESS);
    testOp(">=", JunoPL::BinaryOp::Operation::GREATER_EQ);
    testOp("<=", JunoPL::BinaryOp::Operation::LESS_EQ);
}

TEST_CASE("Parser Math Expression - precedence") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; expr calc() 1 + 2 * 3;");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    const auto &expr = std::get<JunoPL::CustomExpression>(parseOutput.root->body.at(0));
    // Should be + with (1) and (* (2 3))
    const auto &add = std::get<JunoPL::BinaryOp>(expr.expression->value);
    CHECK(add.operation == JunoPL::BinaryOp::Operation::ADD);
    CHECK(std::holds_alternative<JunoPL::Value>(add.lhs->value));
    const auto &mul = std::get<JunoPL::BinaryOp>(add.rhs->value);
    CHECK(mul.operation == JunoPL::BinaryOp::Operation::MULTIPLY);
}

TEST_CASE("Parser Math Expression - modulo") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; expr calc() 10 % 3;");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    const auto &expr = std::get<JunoPL::CustomExpression>(parseOutput.root->body.at(0));
    const auto &binop = std::get<JunoPL::BinaryOp>(expr.expression->value);
    CHECK(binop.operation == JunoPL::BinaryOp::Operation::MOD);
}

TEST_CASE("Parser Unary Expression - not") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; expr check() not true;");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    const auto &expr = std::get<JunoPL::CustomExpression>(parseOutput.root->body.at(0));
    REQUIRE(std::holds_alternative<JunoPL::UnaryOp>(expr.expression->value));

    const auto &unary = std::get<JunoPL::UnaryOp>(expr.expression->value);
    CHECK(unary.operation == JunoPL::UnaryOp::Operation::NOT);
}

TEST_CASE("Parser Unary Expression - negation") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; expr calc() -5;");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    const auto &expr = std::get<JunoPL::CustomExpression>(parseOutput.root->body.at(0));
    REQUIRE(std::holds_alternative<JunoPL::UnaryOp>(expr.expression->value));

    const auto &unary = std::get<JunoPL::UnaryOp>(expr.expression->value);
    CHECK(unary.operation == JunoPL::UnaryOp::Operation::NEGATE);
}

TEST_CASE("Parser Parenthesized Expression") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; expr calc() (1 + 2) * 3;");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    const auto &expr = std::get<JunoPL::CustomExpression>(parseOutput.root->body.at(0));
    // Should be * with (+ 1 2) and 3
    const auto &mul = std::get<JunoPL::BinaryOp>(expr.expression->value);
    CHECK(mul.operation == JunoPL::BinaryOp::Operation::MULTIPLY);
    const auto &add = std::get<JunoPL::BinaryOp>(mul.lhs->value);
    CHECK(add.operation == JunoPL::BinaryOp::Operation::ADD);
}

TEST_CASE("Parser List Operation Expression") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; expr getLen() items.length();");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    const auto &expr = std::get<JunoPL::CustomExpression>(parseOutput.root->body.at(0));
    REQUIRE(std::holds_alternative<JunoPL::ListOp>(expr.expression->value));

    const auto &listOp = std::get<JunoPL::ListOp>(expr.expression->value);
    CHECK(listOp.identifier == "items");
    CHECK(listOp.action == "length");
    CHECK(listOp.params.empty());
}

TEST_CASE("Parser List Index Expression") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; expr getFirst() items[0];");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    const auto &expr = std::get<JunoPL::CustomExpression>(parseOutput.root->body.at(0));
    REQUIRE(std::holds_alternative<JunoPL::ListIndex>(expr.expression->value));

    const auto &listIdx = std::get<JunoPL::ListIndex>(expr.expression->value);
    CHECK(listIdx.identifier == "items");
    CHECK(std::get<JunoPL::Value>(listIdx.index->value).value == "0");
}

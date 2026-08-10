#include "doctest.h"
#include <variant>

import junopl.lexer;
import junopl.parser;
import junopl.parser.nodes;
import junopl.parser.nodes.expressions;
import junopl.parser.nodes.statements;
import junopl.tests.filereader;

TEST_CASE("Parser List Declaration Test") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("list identifier = [1, \"a\", false];");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);

    CHECK(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);
    REQUIRE(parseOutput.root->body.size() == 1);
    REQUIRE(std::holds_alternative<JunoPL::ListDeclaration>(
        parseOutput.root->body.at(0)));

    const auto &listDecl =
        std::get<JunoPL::ListDeclaration>(parseOutput.root->body.at(0));
    CHECK(listDecl.identifier == "identifier");
    REQUIRE(listDecl.values.size() == 3);
    CHECK(std::get<JunoPL::Value>(listDecl.values.at(0)->value).value == "1");
    CHECK(std::get<JunoPL::Value>(listDecl.values.at(1)->value).value == "a");
    CHECK(std::get<JunoPL::Value>(listDecl.values.at(2)->value).value ==
          "false");
}

TEST_CASE("Parser Expression Operators Test") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("expr choose(a, b, c) = not a ? b + 1 : c * 2;");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);

    CHECK(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);
    REQUIRE(parseOutput.root->body.size() == 1);
    REQUIRE(std::holds_alternative<JunoPL::CustomExpression>(
        parseOutput.root->body.at(0)));

    const auto &exprNode =
        std::get<JunoPL::CustomExpression>(parseOutput.root->body.at(0));
    CHECK(exprNode.identifier == "choose");
    REQUIRE(exprNode.expression);

    const auto &ternary = std::get<JunoPL::TernaryOp>(exprNode.expression->value);
    const auto &condition = std::get<JunoPL::UnaryOp>(ternary.condition->value);
    CHECK(condition.operation == JunoPL::UnaryOp::Operation::NOT);

    const auto &trueBranch = std::get<JunoPL::BinaryOp>(ternary.valueIfTrue->value);
    CHECK(trueBranch.operation == JunoPL::BinaryOp::Operation::ADD);

    const auto &falseBranch =
        std::get<JunoPL::BinaryOp>(ternary.valueIfFalse->value);
    CHECK(falseBranch.operation == JunoPL::BinaryOp::Operation::MULTIPLY);
}
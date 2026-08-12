#include "doctest.h"
#include <variant>

import junopl.lexer;
import junopl.lexer.tokens;
import junopl.parser;
import junopl.parser.nodes;
import junopl.parser.nodes.expressions;
import junopl.parser.nodes.statements;
import junopl.parser.handlers;
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

TEST_CASE("Parser Body - empty body") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("{ }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::TokenList tokens;
    for (std::size_t i = 0; i < lexOutput.tokens.size(); ++i) {
        tokens.push_back(lexOutput.tokens.at(i));
    }

    auto result = JunoPL::parseBody(tokens);
    REQUIRE(result.has_value());
    CHECK(result.value().body.empty());
}

TEST_CASE("Parser Body - single var declaration") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("{ var x = 1; }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::TokenList tokens;
    for (std::size_t i = 0; i < lexOutput.tokens.size(); ++i) {
        tokens.push_back(lexOutput.tokens.at(i));
    }

    auto result = JunoPL::parseBody(tokens);
    REQUIRE(result.has_value());
    REQUIRE(result.value().body.size() == 1);
    CHECK(std::holds_alternative<JunoPL::VarDeclaration>(
        result.value().body.at(0)->statement));

    const auto &varDecl = std::get<JunoPL::VarDeclaration>(
        result.value().body.at(0)->statement);
    CHECK(varDecl.identifier == "x");
}

TEST_CASE("Parser Body - multiple statements") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("{ var x = 1; var y = 2; }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::TokenList tokens;
    for (std::size_t i = 0; i < lexOutput.tokens.size(); ++i) {
        tokens.push_back(lexOutput.tokens.at(i));
    }

    auto result = JunoPL::parseBody(tokens);
    REQUIRE(result.has_value());
    REQUIRE(result.value().body.size() == 2);
    CHECK(std::holds_alternative<JunoPL::VarDeclaration>(
        result.value().body.at(0)->statement));
    CHECK(std::holds_alternative<JunoPL::VarDeclaration>(
        result.value().body.at(1)->statement));
}

TEST_CASE("Parser Body - missing opening brace") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("var x = 1; }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::TokenList tokens;
    for (std::size_t i = 0; i < lexOutput.tokens.size(); ++i) {
        tokens.push_back(lexOutput.tokens.at(i));
    }

    auto result = JunoPL::parseBody(tokens);
    CHECK(!result.has_value());
}

TEST_CASE("Parser Body - missing closing brace") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("{ var x = 1;");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::TokenList tokens;
    for (std::size_t i = 0; i < lexOutput.tokens.size(); ++i) {
        tokens.push_back(lexOutput.tokens.at(i));
    }

    auto result = JunoPL::parseBody(tokens);
    CHECK(!result.has_value());
}

TEST_CASE("Parser Body - unexpected token") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("{ 123 }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::TokenList tokens;
    for (std::size_t i = 0; i < lexOutput.tokens.size(); ++i) {
        tokens.push_back(lexOutput.tokens.at(i));
    }

    auto result = JunoPL::parseBody(tokens);
    CHECK(!result.has_value());
}

TEST_CASE("Parser Body - list declaration") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("{ list items = [1, 2, 3]; }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::TokenList tokens;
    for (std::size_t i = 0; i < lexOutput.tokens.size(); ++i) {
        tokens.push_back(lexOutput.tokens.at(i));
    }

    auto result = JunoPL::parseBody(tokens);
    REQUIRE(result.has_value());
    REQUIRE(result.value().body.size() == 1);
    CHECK(std::holds_alternative<JunoPL::ListDeclaration>(
        result.value().body.at(0)->statement));

    const auto &listDecl = std::get<JunoPL::ListDeclaration>(
        result.value().body.at(0)->statement);
    CHECK(listDecl.identifier == "items");
    REQUIRE(listDecl.values.size() == 3);
}
#include "doctest.h"
#include <variant>

import junopl.lexer;
import junopl.lexer.tokens;
import junopl.parser.handlers;
import junopl.parser.nodes.statements;
import junopl.tests.filereader;

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

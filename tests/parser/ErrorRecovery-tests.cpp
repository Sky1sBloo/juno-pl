#include "doctest.h"
#include <string>
#include <variant>

import junopl.lexer;
import junopl.lexer.tokens;
import junopl.parser;
import junopl.parser.nodes;
import junopl.parser.nodes.statements;
import junopl.parser.nodes.expressions;
import junopl.tests.filereader;

TEST_CASE("Error Recovery - multiple errors in body") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { var x = ; var y = 2; var z = ; }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    // Should have errors but still parse the valid statements
    CHECK(!parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    // Should have parsed at least the valid var y = 2
    CHECK(event.body.body.size() >= 1);
}

TEST_CASE("Error Recovery - bad statement then good statement") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { 123 ; var y = 2; }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(!parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    // Should have parsed the valid var y = 2 after recovery
    bool foundVarY = false;
    for (const auto &stmt : event.body.body) {
        if (std::holds_alternative<JunoPL::VarDeclaration>(stmt->statement)) {
            const auto &var = std::get<JunoPL::VarDeclaration>(stmt->statement);
            if (var.identifier == "y") foundVarY = true;
        }
    }
    CHECK(foundVarY);
}

TEST_CASE("Error Recovery - multiple bad statements") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { 123 ; 456 ; var y = 2; }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    // Should have multiple errors
    CHECK(parseOutput.errors.size() >= 2);
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    // Should still parse the valid statement
    CHECK(event.body.body.size() >= 1);
}

TEST_CASE("Error Recovery - recovery to semicolon") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { var x = ; ; var y = 2; }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(!parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    bool foundVarY = false;
    for (const auto &stmt : event.body.body) {
        if (std::holds_alternative<JunoPL::VarDeclaration>(stmt->statement)) {
            const auto &var = std::get<JunoPL::VarDeclaration>(stmt->statement);
            if (var.identifier == "y") foundVarY = true;
        }
    }
    CHECK(foundVarY);
}

TEST_CASE("Error Recovery - bad expression in func call") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { perform doStuff(; var y = 2; }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(!parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    bool foundVarY = false;
    for (const auto &stmt : event.body.body) {
        if (std::holds_alternative<JunoPL::VarDeclaration>(stmt->statement)) {
            const auto &var = std::get<JunoPL::VarDeclaration>(stmt->statement);
            if (var.identifier == "y") foundVarY = true;
        }
    }
    CHECK(foundVarY);
}

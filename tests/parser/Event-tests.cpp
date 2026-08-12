#include "doctest.h"
#include <variant>

import junopl.lexer;
import junopl.lexer.tokens;
import junopl.parser;
import junopl.parser.nodes;
import junopl.parser.nodes.statements;
import junopl.tests.filereader;

TEST_CASE("Parser Event - basic") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);
    REQUIRE(parseOutput.root->body.size() == 1);
    REQUIRE(std::holds_alternative<JunoPL::EventNode>(parseOutput.root->body.at(0)));

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    CHECK(event.identifier == "start");
    CHECK(event.params.empty());
    CHECK(event.body.body.empty());
}

TEST_CASE("Parser Event - with params") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on receive(msg, sender) { }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);
    REQUIRE(parseOutput.root->body.size() == 1);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    CHECK(event.identifier == "receive");
    REQUIRE(event.params.size() == 2);
    CHECK(event.params[0] == "msg");
    CHECK(event.params[1] == "sender");
}

TEST_CASE("Parser Event - with body") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { var x = 1; }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);
    REQUIRE(parseOutput.root->body.size() == 1);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    CHECK(event.identifier == "start");
    REQUIRE(event.body.body.size() == 1);
}

TEST_CASE("Parser Emit - basic") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { emit myEvent(); }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);
    REQUIRE(parseOutput.root->body.size() == 1);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    REQUIRE(event.body.body.size() == 1);
    REQUIRE(std::holds_alternative<JunoPL::EmitEvent>(event.body.body.at(0)->statement));

    const auto &emit = std::get<JunoPL::EmitEvent>(event.body.body.at(0)->statement);
    CHECK(emit.identifier == "myEvent");
    CHECK(emit.params.empty());
    CHECK(emit.option == JunoPL::EmitEvent::TO_SCRIPT);
}

TEST_CASE("Parser Emit - with params") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { emit sendMessage(\"hello\", 42); }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    const auto &emit = std::get<JunoPL::EmitEvent>(event.body.body.at(0)->statement);
    CHECK(emit.identifier == "sendMessage");
    REQUIRE(emit.params.size() == 2);
}

TEST_CASE("Parser Emit - to craft") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { emit ping() to craft; }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    const auto &emit = std::get<JunoPL::EmitEvent>(event.body.body.at(0)->statement);
    CHECK(emit.option == JunoPL::EmitEvent::TO_CRAFT);
}

TEST_CASE("Parser Emit - to nearby") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { emit ping() to nearby; }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    const auto &emit = std::get<JunoPL::EmitEvent>(event.body.body.at(0)->statement);
    CHECK(emit.option == JunoPL::EmitEvent::TO_NEARBY_CRAFTS);
}

TEST_CASE("Parser Emit - qualified identifier") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { emit module.send(); }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    const auto &emit = std::get<JunoPL::EmitEvent>(event.body.body.at(0)->statement);
    CHECK(emit.identifier == "module.send");
}

TEST_CASE("Parser Perform - qualified identifier") {
    JunoPL::Tests::FileReaderTest fileReader;
    fileReader.setSourceCode("program test; on start() { perform module.doStuff(); }");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileReader);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &event = std::get<JunoPL::EventNode>(parseOutput.root->body.at(0));
    const auto &perform = std::get<JunoPL::PerformInstruction>(event.body.body.at(0)->statement);
    CHECK(perform.qualifier == "module");
    CHECK(perform.identifier == "doStuff");
}

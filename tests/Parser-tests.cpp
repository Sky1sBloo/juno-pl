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
#include "doctest.h"
#include <print>

import junopl.tests.filereader;
import junopl.lexer;
import junopl.lexer.tokens;
import junopl.lexer.error;

TEST_CASE("Lexer Number Test") {
    JunoPL::Tests::FileReaderTest fileReader;
    SUBCASE("Integer") {
        fileReader.setSourceCode("1");
        JunoPL::Lexer lexer{fileReader};
        lexer.tokenize();

        CHECK(lexer.getErrors().empty());
        REQUIRE(lexer.getTokens().size() == 1);
        CHECK(lexer.getTokens().at(0).type == JunoPL::TokenType::NUM);
        CHECK(lexer.getTokens().at(0).value == "1");
    }

    SUBCASE("Decimal") {
        fileReader.setSourceCode("1.02");
        JunoPL::Lexer lexer{fileReader};
        lexer.tokenize();

        CHECK(lexer.getErrors().empty());
        REQUIRE(lexer.getTokens().size() == 1);
        CHECK(lexer.getTokens().at(0).type == JunoPL::TokenType::NUM);
        CHECK(lexer.getTokens().at(0).value == "1.02");
    }
}

TEST_CASE("Identifier Test") {
    JunoPL::Tests::FileReaderTest fileReader;
    SUBCASE("Character start") {
        fileReader.setSourceCode("random9vValue");
        JunoPL::Lexer lexer{fileReader};
        lexer.tokenize();

        CHECK(lexer.getErrors().empty());
        REQUIRE(lexer.getTokens().size() == 1);
        CHECK(lexer.getTokens().at(0).type == JunoPL::TokenType::IDENT);
    }

    SUBCASE("Underscore start") {
        fileReader.setSourceCode("_identX");
        JunoPL::Lexer lexer{fileReader};
        lexer.tokenize();

        CHECK(lexer.getErrors().empty());
        REQUIRE(lexer.getTokens().size() == 1);
        CHECK(lexer.getTokens().at(0).type == JunoPL::TokenType::IDENT);
    }

    SUBCASE("Multi") {
        fileReader.setSourceCode("_identX9 r9");
        JunoPL::Lexer lexer{fileReader};
        lexer.tokenize();

        CHECK(lexer.getErrors().empty());
        REQUIRE(lexer.getTokens().size() == 2);
        CHECK(lexer.getTokens().at(0).type == JunoPL::TokenType::IDENT);
        CHECK(lexer.getTokens().at(1).type == JunoPL::TokenType::IDENT);
    }
}

TEST_CASE("Lexer EOF Test") {
    JunoPL::Tests::FileReaderTest fileReader;
    SUBCASE("No whitespace") {
        fileReader.setSourceCode("123");
        JunoPL::Lexer lexer{fileReader};
        lexer.tokenize();

        CHECK(lexer.getErrors().empty());
        REQUIRE(lexer.getTokens().size() == 1);
        CHECK(lexer.getTokens().at(0).type == JunoPL::TokenType::NUM);
    }

    SUBCASE("With whitespace") {
        fileReader.setSourceCode("123 ");
        JunoPL::Lexer lexer{fileReader};
        lexer.tokenize();

        // CHECK(lexer.getErrors().empty());
        for (JunoPL::LexerError error : lexer.getErrors()) {
            std::print("Error: ", error.message());
        }
        REQUIRE(lexer.getTokens().size() == 1);
        CHECK(lexer.getTokens().at(0).type == JunoPL::TokenType::NUM);
    }
}
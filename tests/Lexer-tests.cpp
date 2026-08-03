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

    SUBCASE("Keyword") {
        fileReader.setSourceCode("_identX9 program import");
        JunoPL::Lexer lexer{fileReader};
        lexer.tokenize();

        CHECK(lexer.getErrors().empty());
        REQUIRE(lexer.getTokens().size() == 3);
        CHECK(lexer.getTokens().at(0).type == JunoPL::TokenType::IDENT);
        CHECK(lexer.getTokens().at(1).type == JunoPL::TokenType::K_PROGRAM);
        CHECK(lexer.getTokens().at(2).type == JunoPL::TokenType::K_IMPORT);
    }
}

TEST_CASE("Lexer Operator Test") {
    JunoPL::Tests::FileReaderTest fileReader;

    SUBCASE("Single character operators") {
        fileReader.setSourceCode("+ - * / = ( ) { } % .");
        JunoPL::Lexer lexer{fileReader};
        lexer.tokenize();

        CHECK(lexer.getErrors().empty());
        REQUIRE(lexer.getTokens().size() == 11);
        CHECK(lexer.getTokens().at(0).type == JunoPL::TokenType::OP_PLUS);
        CHECK(lexer.getTokens().at(1).type == JunoPL::TokenType::OP_MINUS);
        CHECK(lexer.getTokens().at(2).type == JunoPL::TokenType::OP_MULT);
        CHECK(lexer.getTokens().at(3).type == JunoPL::TokenType::OP_DIVIDE);
        CHECK(lexer.getTokens().at(4).type == JunoPL::TokenType::OP_EQUAL);
        CHECK(lexer.getTokens().at(5).type == JunoPL::TokenType::OP_PAR_OP);
        CHECK(lexer.getTokens().at(6).type == JunoPL::TokenType::OP_PAR_CLO);
        CHECK(lexer.getTokens().at(7).type == JunoPL::TokenType::OP_BRAC_OP);
        CHECK(lexer.getTokens().at(8).type == JunoPL::TokenType::OP_BRAC_CLO);
        CHECK(lexer.getTokens().at(9).type == JunoPL::TokenType::OP_MOD);
        CHECK(lexer.getTokens().at(10).type == JunoPL::TokenType::OP_DOT);
    }

    SUBCASE("Comparison operators") {
        fileReader.setSourceCode("a<=b>=c<d>e");
        JunoPL::Lexer lexer{fileReader};
        lexer.tokenize();

        CHECK(lexer.getErrors().empty());
        REQUIRE(lexer.getTokens().size() == 9);
        CHECK(lexer.getTokens().at(0).type == JunoPL::TokenType::IDENT);
        CHECK(lexer.getTokens().at(1).type == JunoPL::TokenType::OP_COMP_LESS_EQ);
        CHECK(lexer.getTokens().at(2).type == JunoPL::TokenType::IDENT);
        CHECK(lexer.getTokens().at(3).type == JunoPL::TokenType::OP_COMP_GREATER_EQ);
        CHECK(lexer.getTokens().at(4).type == JunoPL::TokenType::IDENT);
        CHECK(lexer.getTokens().at(5).type == JunoPL::TokenType::OP_COMP_LESS);
        CHECK(lexer.getTokens().at(6).type == JunoPL::TokenType::IDENT);
        CHECK(lexer.getTokens().at(7).type == JunoPL::TokenType::OP_COMP_GREATER);
        CHECK(lexer.getTokens().at(8).type == JunoPL::TokenType::IDENT);
    }
}

TEST_CASE("Lexer String Test") {
    JunoPL::Tests::FileReaderTest fileReader;

    SUBCASE("Simple string") {
        fileReader.setSourceCode("\"hello\"");
        JunoPL::Lexer lexer{fileReader};
        lexer.tokenize();

        CHECK(lexer.getErrors().empty());
        REQUIRE(lexer.getTokens().size() == 1);
        CHECK(lexer.getTokens().at(0).type == JunoPL::TokenType::STR);
        CHECK(lexer.getTokens().at(0).value == "hello");
    }

    SUBCASE("String with spaces and escapes") {
        fileReader.setSourceCode("\"say \\\"hi\\\" now\"");
        JunoPL::Lexer lexer{fileReader};
        lexer.tokenize();

        CHECK(lexer.getErrors().empty());
        REQUIRE(lexer.getTokens().size() == 1);
        CHECK(lexer.getTokens().at(0).type == JunoPL::TokenType::STR);
        CHECK(lexer.getTokens().at(0).value == "say \\\"hi\\\" now");
    }

    SUBCASE("Unclosed string") {
        fileReader.setSourceCode("\"missing end");
        JunoPL::Lexer lexer{fileReader};
        lexer.tokenize();

        CHECK_FALSE(lexer.getErrors().empty());
        REQUIRE(lexer.getTokens().empty());
        CHECK(lexer.getErrors().at(0).type() ==
              JunoPL::LexerError::Type::UnclosedToken);
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
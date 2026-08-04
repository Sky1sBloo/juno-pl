#include "doctest.h"

import junopl.tests.filereader;
import junopl.lexer;
import junopl.lexer.tokens;
import junopl.lexer.error;

TEST_CASE("Lexer Number Test") {
    JunoPL::Tests::FileReaderTest fileReader;
    SUBCASE("Integer") {
        fileReader.setSourceCode("1");
        JunoPL::Lexer lexer;
        JunoPL::Lexer::Output output = lexer.tokenize(fileReader);

        CHECK(output.errors.empty());
        REQUIRE(output.tokens.size() == 1);
        CHECK(output.tokens.at(0).type == JunoPL::TokenType::NUM);
        CHECK(output.tokens.at(0).value == "1");
    }

    SUBCASE("Decimal") {
        fileReader.setSourceCode("1.02");
        JunoPL::Lexer lexer;
        JunoPL::Lexer::Output output = lexer.tokenize(fileReader);

        CHECK(output.errors.empty());
        REQUIRE(output.tokens.size() == 1);
        CHECK(output.tokens.at(0).type == JunoPL::TokenType::NUM);
        CHECK(output.tokens.at(0).value == "1.02");
    }
}

TEST_CASE("Identifier Test") {
    JunoPL::Tests::FileReaderTest fileReader;
    SUBCASE("Character start") {
        fileReader.setSourceCode("random9vValue");
        JunoPL::Lexer lexer;
        JunoPL::Lexer::Output output = lexer.tokenize(fileReader);

        CHECK(output.errors.empty());
        REQUIRE(output.tokens.size() == 1);
        CHECK(output.tokens.at(0).type == JunoPL::TokenType::IDENT);
    }

    SUBCASE("Underscore start") {
        fileReader.setSourceCode("_identX");
        JunoPL::Lexer lexer;
        JunoPL::Lexer::Output output = lexer.tokenize(fileReader);

        CHECK(output.errors.empty());
        REQUIRE(output.tokens.size() == 1);
        CHECK(output.tokens.at(0).type == JunoPL::TokenType::IDENT);
    }

    SUBCASE("Multi") {
        fileReader.setSourceCode("_identX9 r9");
        JunoPL::Lexer lexer;
        JunoPL::Lexer::Output output = lexer.tokenize(fileReader);

        CHECK(output.errors.empty());
        REQUIRE(output.tokens.size() == 2);
        CHECK(output.tokens.at(0).type == JunoPL::TokenType::IDENT);
        CHECK(output.tokens.at(1).type == JunoPL::TokenType::IDENT);
    }

    SUBCASE("Keyword") {
        fileReader.setSourceCode("_identX9 program import");
        JunoPL::Lexer lexer;
        JunoPL::Lexer::Output output = lexer.tokenize(fileReader);

        CHECK(output.errors.empty());
        REQUIRE(output.tokens.size() == 3);
        CHECK(output.tokens.at(0).type == JunoPL::TokenType::IDENT);
        CHECK(output.tokens.at(1).type == JunoPL::TokenType::K_PROGRAM);
        CHECK(output.tokens.at(2).type == JunoPL::TokenType::K_IMPORT);
    }
}

TEST_CASE("Lexer Operator Test") {
    JunoPL::Tests::FileReaderTest fileReader;

    SUBCASE("Single character operators") {
        fileReader.setSourceCode("+ - * / = ( ) { } % .");
        JunoPL::Lexer lexer;
        JunoPL::Lexer::Output output = lexer.tokenize(fileReader);

        CHECK(output.errors.empty());
        REQUIRE(output.tokens.size() == 11);
        CHECK(output.tokens.at(0).type == JunoPL::TokenType::OP_PLUS);
        CHECK(output.tokens.at(1).type == JunoPL::TokenType::OP_MINUS);
        CHECK(output.tokens.at(2).type == JunoPL::TokenType::OP_MULT);
        CHECK(output.tokens.at(3).type == JunoPL::TokenType::OP_DIVIDE);
        CHECK(output.tokens.at(4).type == JunoPL::TokenType::OP_EQUAL);
        CHECK(output.tokens.at(5).type == JunoPL::TokenType::OP_PAR_OP);
        CHECK(output.tokens.at(6).type == JunoPL::TokenType::OP_PAR_CLO);
        CHECK(output.tokens.at(7).type == JunoPL::TokenType::OP_BRAC_OP);
        CHECK(output.tokens.at(8).type == JunoPL::TokenType::OP_BRAC_CLO);
        CHECK(output.tokens.at(9).type == JunoPL::TokenType::OP_MOD);
        CHECK(output.tokens.at(10).type == JunoPL::TokenType::OP_DOT);
    }

    SUBCASE("Comparison operators") {
        fileReader.setSourceCode("a<=b>=c<d>e");
        JunoPL::Lexer lexer;
        JunoPL::Lexer::Output output = lexer.tokenize(fileReader);

        CHECK(output.errors.empty());
        REQUIRE(output.tokens.size() == 9);
        CHECK(output.tokens.at(0).type == JunoPL::TokenType::IDENT);
        CHECK(output.tokens.at(1).type == JunoPL::TokenType::OP_COMP_LESS_EQ);
        CHECK(output.tokens.at(2).type == JunoPL::TokenType::IDENT);
        CHECK(output.tokens.at(3).type ==
              JunoPL::TokenType::OP_COMP_GREATER_EQ);
        CHECK(output.tokens.at(4).type == JunoPL::TokenType::IDENT);
        CHECK(output.tokens.at(5).type == JunoPL::TokenType::OP_COMP_LESS);
        CHECK(output.tokens.at(6).type == JunoPL::TokenType::IDENT);
        CHECK(output.tokens.at(7).type == JunoPL::TokenType::OP_COMP_GREATER);
        CHECK(output.tokens.at(8).type == JunoPL::TokenType::IDENT);
    }
}

TEST_CASE("Lexer String Test") {
    JunoPL::Tests::FileReaderTest fileReader;

    SUBCASE("Simple string") {
        fileReader.setSourceCode("\"hello\"");
        JunoPL::Lexer lexer;
        JunoPL::Lexer::Output output = lexer.tokenize(fileReader);

        CHECK(output.errors.empty());
        REQUIRE(output.tokens.size() == 1);
        CHECK(output.tokens.at(0).type == JunoPL::TokenType::STR);
        CHECK(output.tokens.at(0).value == "hello");
    }

    SUBCASE("String with spaces and escapes") {
        fileReader.setSourceCode("\"say \\\"hi\\\" now\"");
        JunoPL::Lexer lexer;
        JunoPL::Lexer::Output output = lexer.tokenize(fileReader);

        CHECK(output.errors.empty());
        REQUIRE(output.tokens.size() == 1);
        CHECK(output.tokens.at(0).type == JunoPL::TokenType::STR);
        CHECK(output.tokens.at(0).value == "say \\\"hi\\\" now");
    }

    SUBCASE("Unclosed string") {
        fileReader.setSourceCode("\"missing end");
        JunoPL::Lexer lexer;
        JunoPL::Lexer::Output output = lexer.tokenize(fileReader);

        CHECK_FALSE(output.errors.empty());
        REQUIRE(output.tokens.empty());
        CHECK(output.errors.at(0).type() ==
              JunoPL::LexerError::Type::UnclosedToken);
    }
}

TEST_CASE("Lexer EOF Test") {
    JunoPL::Tests::FileReaderTest fileReader;
    SUBCASE("No whitespace") {
        fileReader.setSourceCode("123");
        JunoPL::Lexer lexer;
        JunoPL::Lexer::Output output = lexer.tokenize(fileReader);

        CHECK(output.errors.empty());
        REQUIRE(output.tokens.size() == 1);
        CHECK(output.tokens.at(0).type == JunoPL::TokenType::NUM);
    }

    SUBCASE("With whitespace") {
        fileReader.setSourceCode("123 ");
        JunoPL::Lexer lexer;
        JunoPL::Lexer::Output output = lexer.tokenize(fileReader);

        CHECK(output.errors.empty());
        REQUIRE(output.tokens.size() == 1);
        CHECK(output.tokens.at(0).type == JunoPL::TokenType::NUM);
    }
}
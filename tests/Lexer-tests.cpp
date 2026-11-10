#include "doctest.h"
#include <string>

import junopl.tests.filehandler;
import junopl.lexer;
import junopl.lexer.tokens;
import junopl.lexer.error;

TEST_CASE("Lexer Number Test") {
    JunoPL::Tests::FileHandlerTest fileHandler;
    SUBCASE("Integer") {
        fileHandler.setSourceCode("1");
        JunoPL::Lexer lexer;
        JunoPL::Lexer::Output output = lexer.tokenize(fileHandler);

        CHECK(output.errors.empty());
        REQUIRE(output.tokens.size() == 1);
        CHECK(output.tokens.at(0).type == JunoPL::TokenType::NUM);
        CHECK(output.tokens.at(0).value == "1");
    }

    SUBCASE("Decimal") {
        fileHandler.setSourceCode("1.02");
        JunoPL::Lexer lexer;
        JunoPL::Lexer::Output output = lexer.tokenize(fileHandler);

        CHECK(output.errors.empty());
        REQUIRE(output.tokens.size() == 1);
        CHECK(output.tokens.at(0).type == JunoPL::TokenType::NUM);
        CHECK(output.tokens.at(0).value == "1.02");
    }
}

TEST_CASE("Identifier Test") {
    JunoPL::Tests::FileHandlerTest fileHandler;
    SUBCASE("Character start") {
        fileHandler.setSourceCode("random9vValue");
        JunoPL::Lexer lexer;
        JunoPL::Lexer::Output output = lexer.tokenize(fileHandler);

        CHECK(output.errors.empty());
        REQUIRE(output.tokens.size() == 1);
        CHECK(output.tokens.at(0).type == JunoPL::TokenType::IDENT);
    }

    SUBCASE("Underscore start") {
        fileHandler.setSourceCode("_identX");
        JunoPL::Lexer lexer;
        JunoPL::Lexer::Output output = lexer.tokenize(fileHandler);

        CHECK(output.errors.empty());
        REQUIRE(output.tokens.size() == 1);
        CHECK(output.tokens.at(0).type == JunoPL::TokenType::IDENT);
    }

    SUBCASE("Multi") {
        fileHandler.setSourceCode("_identX9 r9");
        JunoPL::Lexer lexer;
        JunoPL::Lexer::Output output = lexer.tokenize(fileHandler);

        CHECK(output.errors.empty());
        REQUIRE(output.tokens.size() == 2);
        CHECK(output.tokens.at(0).type == JunoPL::TokenType::IDENT);
        CHECK(output.tokens.at(1).type == JunoPL::TokenType::IDENT);
    }

    SUBCASE("Keyword") {
        fileHandler.setSourceCode("_identX9 program import");
        JunoPL::Lexer lexer;
        JunoPL::Lexer::Output output = lexer.tokenize(fileHandler);

        CHECK(output.errors.empty());
        REQUIRE(output.tokens.size() == 3);
        CHECK(output.tokens.at(0).type == JunoPL::TokenType::IDENT);
        CHECK(output.tokens.at(1).type == JunoPL::TokenType::K_PROGRAM);
        CHECK(output.tokens.at(2).type == JunoPL::TokenType::K_IMPORT);
    }
}

TEST_CASE("Lexer Operator Test") {
    JunoPL::Tests::FileHandlerTest fileHandler;

    SUBCASE("Single character operators") {
        fileHandler.setSourceCode("+ - * / = ; ( ) [ ] { } % . : ?");
        JunoPL::Lexer lexer;
        JunoPL::Lexer::Output output = lexer.tokenize(fileHandler);

        CHECK(output.errors.empty());
        REQUIRE(output.tokens.size() == 16);
        CHECK(output.tokens.at(0).type == JunoPL::TokenType::OP_PLUS);
        CHECK(output.tokens.at(1).type == JunoPL::TokenType::OP_MINUS);
        CHECK(output.tokens.at(2).type == JunoPL::TokenType::OP_MULT);
        CHECK(output.tokens.at(3).type == JunoPL::TokenType::OP_DIVIDE);
        CHECK(output.tokens.at(4).type == JunoPL::TokenType::OP_EQUAL);
        CHECK(output.tokens.at(5).type == JunoPL::TokenType::OP_SEMICOLON);
        CHECK(output.tokens.at(6).type == JunoPL::TokenType::OP_PAR_OP);
        CHECK(output.tokens.at(7).type == JunoPL::TokenType::OP_PAR_CLO);
        CHECK(output.tokens.at(8).type == JunoPL::TokenType::OP_BRAC_OP);
        CHECK(output.tokens.at(9).type == JunoPL::TokenType::OP_BRAC_CLO);
        CHECK(output.tokens.at(10).type == JunoPL::TokenType::OP_CBRAC_OP);
        CHECK(output.tokens.at(11).type == JunoPL::TokenType::OP_CBRAC_CLO);
        CHECK(output.tokens.at(12).type == JunoPL::TokenType::OP_MOD);
        CHECK(output.tokens.at(13).type == JunoPL::TokenType::OP_DOT);
        CHECK(output.tokens.at(14).type == JunoPL::TokenType::OP_COLON);
        CHECK(output.tokens.at(15).type == JunoPL::TokenType::OP_QUESTION);
    }

    SUBCASE("Comparison operators") {
        fileHandler.setSourceCode("a<=b>=c<d>e");
        JunoPL::Lexer lexer;
        JunoPL::Lexer::Output output = lexer.tokenize(fileHandler);

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
    JunoPL::Tests::FileHandlerTest fileHandler;

    SUBCASE("Simple string") {
        fileHandler.setSourceCode("\"hello\"");
        JunoPL::Lexer lexer;
        JunoPL::Lexer::Output output = lexer.tokenize(fileHandler);

        CHECK(output.errors.empty());
        REQUIRE(output.tokens.size() == 1);
        CHECK(output.tokens.at(0).type == JunoPL::TokenType::STR);
        CHECK(output.tokens.at(0).value == "hello");
    }

    SUBCASE("String with spaces and escapes") {
        fileHandler.setSourceCode("\"say \\\"hi\\\" now\"");
        JunoPL::Lexer lexer;
        JunoPL::Lexer::Output output = lexer.tokenize(fileHandler);

        CHECK(output.errors.empty());
        REQUIRE(output.tokens.size() == 1);
        CHECK(output.tokens.at(0).type == JunoPL::TokenType::STR);
        CHECK(output.tokens.at(0).value == "say \\\"hi\\\" now");
    }

    SUBCASE("Unclosed string") {
        fileHandler.setSourceCode("\"missing end");
        JunoPL::Lexer lexer;
        JunoPL::Lexer::Output output = lexer.tokenize(fileHandler);

        CHECK_FALSE(output.errors.empty());
        REQUIRE(output.tokens.empty());
        CHECK(output.errors.at(0).type() ==
              JunoPL::LexerError::Type::UnclosedToken);
    }
}

TEST_CASE("Lexer EOF Test") {
    JunoPL::Tests::FileHandlerTest fileHandler;
    SUBCASE("No whitespace") {
        fileHandler.setSourceCode("123");
        JunoPL::Lexer lexer;
        JunoPL::Lexer::Output output = lexer.tokenize(fileHandler);

        CHECK(output.errors.empty());
        REQUIRE(output.tokens.size() == 1);
        CHECK(output.tokens.at(0).type == JunoPL::TokenType::NUM);
    }

    SUBCASE("With whitespace") {
        fileHandler.setSourceCode("123 ");
        JunoPL::Lexer lexer;
        JunoPL::Lexer::Output output = lexer.tokenize(fileHandler);

        CHECK(output.errors.empty());
        REQUIRE(output.tokens.size() == 1);
        CHECK(output.tokens.at(0).type == JunoPL::TokenType::NUM);
    }
}
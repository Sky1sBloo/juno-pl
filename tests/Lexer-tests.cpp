#include "doctest.h"

import junopl.tests.filereader;
import junopl.lexer;
import junopl.lexer.tokens;

TEST_CASE("Lexer single token test") {
    JunoPL::Tests::FileReaderTest fileReader;
    SUBCASE("Integer") {
        fileReader.setSourceCode("1");
        JunoPL::Lexer lexer{fileReader};
        lexer.tokenize();
        CHECK(lexer.getErrors().empty());
        REQUIRE(lexer.getTokens().size() == 1);
        CHECK(lexer.getTokens().at(0).type == JunoPL::TokenType::NUM);
    }
} 
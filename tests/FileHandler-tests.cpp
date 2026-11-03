#include "doctest.h"
#include <string>

import junopl.tests.filehandler;

TEST_CASE("File Reader Test") {
    std::string testStr = "123";
    JunoPL::Tests::FileHandlerTest fileHandler{testStr};
    for (char c : testStr) {
        auto value = fileHandler.read();
        REQUIRE(value.has_value());
        REQUIRE(value.value() == c);
    }
}


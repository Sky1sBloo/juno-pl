#include "doctest.h"
#include <string>

import junopl.tests.filereader;

TEST_CASE("File Reader Test") {
    std::string testStr = "123";
    JunoPL::Tests::FileReaderTest fileReader{testStr};
    for (char c : testStr) {
        auto value = fileReader.read();
        REQUIRE(value.has_value());
        REQUIRE(value.value() == c);
    }
}


#include "doctest.h"
#include <filesystem>
#include <fstream>
#include <string>

import junopl.config;

namespace {
namespace fs = std::filesystem;

/**
Builds a throwaway directory tree and removes it again, so the walk-up
search can be exercised against a real filesystem.
 */
class TempTree {
  public:
    TempTree() {
        mRoot = fs::weakly_canonical(fs::temp_directory_path()) /
                "juno-pl-config-tests";
        std::error_code ec;
        fs::remove_all(mRoot, ec);
        fs::create_directories(mRoot);
    }

    ~TempTree() {
        std::error_code ec;
        fs::remove_all(mRoot, ec);
    }

    TempTree(const TempTree &) = delete;
    TempTree &operator=(const TempTree &) = delete;

    const fs::path &root() const { return mRoot; }

    void write(const std::string &relative, const std::string &contents) {
        const fs::path path = mRoot / relative;
        std::error_code ec;
        fs::create_directories(path.parent_path(), ec);
        std::ofstream out(path);
        out << contents;
    }

  private:
    fs::path mRoot;
};
} // namespace

TEST_CASE("Config - finds a file in a parent directory") {
    TempTree tree;
    tree.write("config.json", "{}");
    tree.write("project/sub/deep/keep.txt", "");

    const auto found =
        JunoPL::findFileUpwards(tree.root() / "project/sub/deep", "config.json");
    REQUIRE(found.has_value());
    CHECK(*found == tree.root() / "config.json");
}

TEST_CASE("Config - missing file yields nothing") {
    TempTree tree;
    tree.write("project/keep.txt", "");

    const auto found = JunoPL::findFileUpwards(tree.root() / "project",
                                               "main.juno");
    CHECK(!found.has_value());
}

TEST_CASE("Config - absent config is not an error") {
    TempTree tree;
    tree.write("project/keep.txt", "");

    const auto config = JunoPL::loadConfig(tree.root() / "project");
    REQUIRE(config.has_value());
    CHECK(config->path.empty());
    CHECK(config->outputDir.empty());
    CHECK(config->outputFile.empty());
}

TEST_CASE("Config - relative outputDir resolves against the config") {
    TempTree tree;
    tree.write("config.json", R"({"outputDir": "build/xml"})");
    tree.write("project/sub/keep.txt", "");

    const auto config = JunoPL::loadConfig(tree.root() / "project/sub");
    REQUIRE(config.has_value());
    CHECK(config->path == tree.root() / "config.json");
    CHECK(config->outputDir == tree.root() / "build/xml");
}

TEST_CASE("Config - absolute outputDir is kept as written") {
    TempTree tree;
    const fs::path elsewhere =
        fs::weakly_canonical(fs::temp_directory_path()) / "elsewhere";
    tree.write("config.json",
               // generic_string() keeps the JSON valid on Windows, where
               // the native spelling uses backslashes.
               R"({"outputDir": ")" + elsewhere.generic_string() + R"("})");

    const auto config = JunoPL::loadConfig(tree.root());
    REQUIRE(config.has_value());
    CHECK(config->outputDir == elsewhere);
}

TEST_CASE("Config - outputFile is read as written") {
    TempTree tree;
    tree.write("config.json",
               R"({"outputDir": "build/xml", "outputFile": "custom.xml"})");

    const auto config = JunoPL::loadConfig(tree.root());
    REQUIRE(config.has_value());
    CHECK(config->outputFile == "custom.xml");
}

TEST_CASE("Config - outputFile must be a string") {
    TempTree tree;
    tree.write("config.json",
               R"({"outputDir": "build/xml", "outputFile": 12})");

    const auto config = JunoPL::loadConfig(tree.root());
    REQUIRE(!config.has_value());
    CHECK(config.error().find("outputFile") != std::string::npos);
}

TEST_CASE("Config - output file name defaults to the program name") {
    const JunoPL::Config empty;
    CHECK(JunoPL::outputFileName(empty, "myApp") == "myApp.xml");

    JunoPL::Config configured;
    configured.outputFile = "custom.xml";
    CHECK(JunoPL::outputFileName(configured, "myApp") == "custom.xml");
}

TEST_CASE("Config - malformed json reports the file") {
    TempTree tree;
    tree.write("config.json", "{ not json");

    const auto config = JunoPL::loadConfig(tree.root());
    REQUIRE(!config.has_value());
    CHECK(config.error().find("config.json") != std::string::npos);
}

TEST_CASE("Config - outputDir must be a string") {
    TempTree tree;
    tree.write("config.json", R"({"outputDir": 12})");

    const auto config = JunoPL::loadConfig(tree.root());
    REQUIRE(!config.has_value());
    CHECK(config.error().find("outputDir") != std::string::npos);
}

TEST_CASE("Config - finds main.juno from a nested directory") {
    TempTree tree;
    tree.write("project/main.juno", "program test;");

    const auto found = JunoPL::findFileUpwards(tree.root() / "project",
                                               "main.juno");
    REQUIRE(found.has_value());
    CHECK(*found == tree.root() / "project/main.juno");
}

TEST_CASE("Config - output directory precedence") {
    const JunoPL::Config empty;
    const fs::path source = "/project/src/main.juno";

    // -d wins over everything
    CHECK(JunoPL::outputDirectory("/cli/dir", empty, source) == "/cli/dir");

    // the config is used when -d is absent
    JunoPL::Config configured;
    configured.outputDir = "/from/config";
    CHECK(JunoPL::outputDirectory("", configured, source) == "/from/config");

    // without either, the XML lands next to the source
    CHECK(JunoPL::outputDirectory("", empty, source) == "/project/src");
}

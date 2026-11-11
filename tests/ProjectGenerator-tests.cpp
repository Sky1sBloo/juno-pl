#include "doctest.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

import junopl.projectgenerator;

namespace {
namespace fs = std::filesystem;

/** A throwaway directory tree that removes itself afterwards. */
class TempTree {
  public:
    TempTree() {
        mRoot = fs::weakly_canonical(fs::temp_directory_path()) /
                "juno-pl-project-generator-tests";
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
        std::ofstream out(path, std::ios::binary);
        out << contents;
    }

    std::string read(const std::string &relative) const {
        std::ifstream in(mRoot / relative, std::ios::binary);
        std::ostringstream contents;
        contents << in.rdbuf();
        return contents.str();
    }

    bool exists(const std::string &relative) const {
        std::error_code ec;
        return fs::exists(mRoot / relative, ec);
    }

  private:
    fs::path mRoot;
};
} // namespace

TEST_CASE("Project generator - accepts Juno identifiers") {
    CHECK(JunoPL::isValidProjectName("hello"));
    CHECK(JunoPL::isValidProjectName("HelloWorld"));
    CHECK(JunoPL::isValidProjectName("_private"));
    CHECK(JunoPL::isValidProjectName("project2"));
}

TEST_CASE("Project generator - rejects invalid project names") {
    CHECK(!JunoPL::isValidProjectName(""));
    CHECK(!JunoPL::isValidProjectName("2fast"));
    CHECK(!JunoPL::isValidProjectName("with-dash"));
    CHECK(!JunoPL::isValidProjectName("has space"));
    CHECK(!JunoPL::isValidProjectName("../escape"));
}

TEST_CASE("Project generator - writes main.juno and config.json") {
    TempTree tree;

    const auto generated = JunoPL::generateProject(tree.root(), "demo");
    REQUIRE(generated.has_value());
    CHECK(*generated == tree.root() / "demo");

    const std::string mainSource = tree.read("demo/main.juno");
    CHECK(mainSource.find("program demo;") != std::string::npos);
    CHECK(mainSource.find("{{project}}") == std::string::npos);

    const std::string config = tree.read("demo/config.json");
    CHECK(config.find("\"outputFile\": \"demo.xml\"") != std::string::npos);
    CHECK(config.find("{{project}}") == std::string::npos);
}

TEST_CASE("Project generator - invalid name creates nothing") {
    TempTree tree;

    const auto generated = JunoPL::generateProject(tree.root(), "../escape");
    CHECK(!generated.has_value());
    CHECK(generated.error().find("invalid project name") != std::string::npos);
    CHECK(!tree.exists("escape/main.juno"));
}

TEST_CASE("Project generator - refuses to overwrite an existing project") {
    TempTree tree;

    REQUIRE(JunoPL::generateProject(tree.root(), "demo").has_value());
    tree.write("demo/main.juno", "mine");

    const auto generated = JunoPL::generateProject(tree.root(), "demo");
    CHECK(!generated.has_value());
    CHECK(generated.error().find("refusing to overwrite") != std::string::npos);
    CHECK(tree.read("demo/main.juno") == "mine");
}

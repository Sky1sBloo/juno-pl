module;
#include <expected>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <system_error>
#include <json.hpp>
module junopl.config;

namespace JunoPL {
namespace fs = std::filesystem;

std::optional<fs::path> findFileUpwards(const fs::path &startDir,
                                        const std::string &name) {
    std::error_code ec;
    fs::path dir = fs::weakly_canonical(startDir, ec);
    if (ec) {
        dir = startDir;
    }

    while (true) {
        std::error_code probe;
        if (fs::is_regular_file(dir / name, probe)) {
            return dir / name;
        }
        const fs::path parent = dir.parent_path();
        if (parent == dir) {
            return std::nullopt; // reached the root
        }
        dir = parent;
    }
}

std::expected<Config, std::string>
loadConfig(const fs::path &startDir) {
    const auto found = findFileUpwards(startDir, "config.json");
    if (!found) {
        return Config{};
    }

    std::ifstream in(*found);
    if (!in) {
        return std::unexpected("cannot read " + found->string());
    }
    std::ostringstream contents;
    contents << in.rdbuf();

    nlohmann::json root;
    try {
        root = nlohmann::json::parse(contents.str());
    } catch (const nlohmann::json::parse_error &error) {
        return std::unexpected(found->string() + ": " + error.what());
    }

    if (!root.is_object()) {
        return std::unexpected(found->string() +
                               ": expected a JSON object at the top level");
    }
    if (!root.contains("outputDir") || !root.at("outputDir").is_string()) {
        return std::unexpected(found->string() +
                               ": expected a string value for \"outputDir\"");
    }

    Config config;
    config.path = *found;
    config.outputDir = root.at("outputDir").get<std::string>();
    if (config.outputDir.is_relative()) {

        config.outputDir = config.path.parent_path() / config.outputDir;
    }
    return config;
}

fs::path outputDirectory(const fs::path &cliDir, const Config &config,
                         const fs::path &sourcePath) {
    if (!cliDir.empty()) {
        return cliDir;
    }
    if (!config.outputDir.empty()) {
        return config.outputDir;
    }
    return sourcePath.parent_path();
}
} 
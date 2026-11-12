module;
#include <cstdlib>
#ifdef _WIN32
#include <cwchar>
#endif
#include <expected>
#include <filesystem>
#include <fstream>
#include <json.hpp>
#include <optional>
#include <sstream>
#include <string>
#include <system_error>
module junopl.config;

namespace JunoPL {
namespace fs = std::filesystem;

// Expands leading `~` to the user's home directory ($HOME on Unix,
// %USERPROFILE% on Windows) and %VAR% environment placeholders (e.g.
// %APPDATA% on Windows). Paths that need no expansion are returned
// unchanged. `~user` (another user's home) is not supported.
fs::path expandPath(fs::path p) {
    using string_type = fs::path::string_type;
    using value_type = fs::path::value_type;

    // Expand a leading `~` to the home directory.
    if (!p.empty() && p.native()[0] == static_cast<value_type>('~')) {
        const char *home = std::getenv("HOME");
        if (!home || *home == '\0') {
            home = std::getenv("USERPROFILE");
        }
        if (home && *home != '\0') {
            fs::path result(home);
            const string_type &native = p.native();
            std::size_t i = 1;
            while (i < native.size() &&
                   (native[i] == static_cast<value_type>('/') ||
                    native[i] == static_cast<value_type>('\\'))) {
                ++i;
            }
            if (i < native.size()) {
                result /= fs::path(native.substr(i));
            }
            return result;
        }
        return p;
    }

    // Expand %VAR% environment variable placeholders.
    const string_type &native = p.native();
    string_type result;
    result.reserve(native.size());
    for (std::size_t i = 0; i < native.size(); ++i) {
        if (native[i] == static_cast<value_type>('%')) {
            const std::size_t end =
                native.find(static_cast<value_type>('%'), i + 1);
            if (end != string_type::npos) {
                const string_type var = native.substr(i + 1, end - i - 1);
                fs::path value;
#ifdef _WIN32
                if (const wchar_t *wide = _wgetenv(var.c_str())) {
                    value = fs::path(wide);
                }
#else
                if (const char *narrow = std::getenv(var.c_str())) {
                    value = fs::path(narrow);
                }
#endif
                if (!value.empty()) {
                    result += value.native();
                } else {
                    result.append(native, i, end - i + 1);
                }
                i = end;
                continue;
            }
        }
        result += native[i];
    }
    return fs::path(result);
}

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

std::expected<Config, std::string> loadConfig(const fs::path &startDir) {
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

    if (root.contains("outputFile") && !root.at("outputFile").is_string()) {
        return std::unexpected(found->string() +
                               ": expected a string value for \"outputFile\"");
    }

    Config config;
    config.path = *found;
    config.outputDir = expandPath(root.at("outputDir").get<std::string>());
    if (root.contains("outputFile")) {
        config.outputFile = root.at("outputFile").get<std::string>();
    }
    if (config.outputDir.is_relative()) {

        config.outputDir = config.path.parent_path() / config.outputDir;
    }
    return config;
}

fs::path outputFileName(const Config &config, const std::string &programName) {
    if (!config.outputFile.empty()) {
        return config.outputFile;
    }
    return programName + ".xml";
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
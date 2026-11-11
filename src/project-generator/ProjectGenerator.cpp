module;

#include "EmbeddedTemplates.hpp"
#include <cctype>
#include <expected>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <system_error>

module junopl.projectgenerator;

namespace JunoPL {
namespace fs = std::filesystem;

namespace {
constexpr std::string_view projectPlaceholder = "{{project}}";

std::string_view configTemplate() {
#if defined(_WIN32)
    return EmbeddedTemplates::configWindows;
#else
    return EmbeddedTemplates::configLinux;
#endif
}

void replaceAll(std::string &text, std::string_view needle,
                std::string_view replacement) {
    std::size_t position = 0;
    while ((position = text.find(needle, position)) != std::string::npos) {
        text.replace(position, needle.size(), replacement);
        position += replacement.size();
    }
}

std::expected<void, std::string> writeFile(const fs::path &path,
                                           const std::string &contents) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        return std::unexpected("cannot create '" + path.string() + "'");
    }
    out.write(contents.data(), static_cast<std::streamsize>(contents.size()));
    out.close();
    if (!out) {
        return std::unexpected("cannot write '" + path.string() + "'");
    }
    return {};
}
} // namespace

bool isValidProjectName(const std::string &name) {
    if (name.empty()) {
        return false;
    }

    const auto isStart = [](unsigned char c) {
        return std::isalpha(c) != 0 || c == '_';
    };
    const auto isPart = [](unsigned char c) {
        return std::isalnum(c) != 0 || c == '_';
    };

    if (!isStart(static_cast<unsigned char>(name.front()))) {
        return false;
    }
    for (const char c : name) {
        if (!isPart(static_cast<unsigned char>(c))) {
            return false;
        }
    }
    return true;
}

std::expected<fs::path, std::string>
generateProject(const fs::path &parentDir, const std::string &projectName) {
    if (!isValidProjectName(projectName)) {
        return std::unexpected(
            "invalid project name '" + projectName +
            "': use a letter or underscore followed by letters, digits, or "
            "underscores");
    }

    std::string mainSource(EmbeddedTemplates::main);
    std::string configSource(configTemplate());
    replaceAll(mainSource, projectPlaceholder, projectName);
    replaceAll(configSource, projectPlaceholder, projectName);

    const fs::path projectDir = parentDir / projectName;
    std::error_code ec;
    fs::create_directories(projectDir, ec);
    if (ec) {
        return std::unexpected("cannot create '" + projectDir.string() +
                               "': " + ec.message());
    }

    const fs::path mainPath = projectDir / "main.juno";
    const fs::path configPath = projectDir / "config.json";
    if (fs::exists(mainPath, ec) || fs::exists(configPath, ec)) {
        return std::unexpected("refusing to overwrite the existing project '" +
                               projectDir.string() + "'");
    }

    auto mainWrite = writeFile(mainPath, mainSource);
    if (!mainWrite) {
        return std::unexpected(mainWrite.error());
    }
    auto configWrite = writeFile(configPath, configSource);
    if (!configWrite) {
        return std::unexpected(configWrite.error());
    }

    return projectDir;
}
} // namespace JunoPL

module;

#include <expected>
#include <filesystem>
#include <optional>
#include <string>

export module junopl.config;

namespace JunoPL {
/**
Project configuration, read from a config.json that is searched for in the
current directory and then in every parent, so a project config applies
from anywhere inside the project.
 */
export struct Config {
    // Where the compiled XML goes, exactly as the config resolved it.
    // Empty when there is no config.
    std::filesystem::path outputDir;
    // File name for the compiled XML, exactly as written in the config.
    // Empty when the config does not set one.
    std::filesystem::path outputFile;
    // The config file it came from, empty when there is none. Relative
    // outputDir values are resolved against this file's directory.
    std::filesystem::path path;
};

/**
Looks for `name` in startDir and then in each parent directory, stopping
at the filesystem root. Returns nothing when the file does not exist.
 */
export std::optional<std::filesystem::path>
findFileUpwards(const std::filesystem::path &startDir,
                const std::string &name);

/**
Reads config.json from startDir or any parent directory. A missing file
is not an error and yields an empty Config; a file that cannot be parsed
or that lacks the outputDir string yields an error message instead.
 */
export std::expected<Config, std::string>
loadConfig(const std::filesystem::path &startDir);

/**
The file name the compiled XML is written as: the config's outputFile when
set, otherwise `<programName>.xml`.
 */
export std::filesystem::path
outputFileName(const Config &config, const std::string &programName);

/**
The directory the output belongs in: the one named on the command line,
otherwise the one from the config, otherwise the source file's own
directory.
 */
export std::filesystem::path
outputDirectory(const std::filesystem::path &cliDir, const Config &config,
                const std::filesystem::path &sourcePath);
} // namespace JunoPL

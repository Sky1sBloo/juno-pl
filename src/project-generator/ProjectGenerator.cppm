module;

#include <expected>
#include <filesystem>
#include <string>

export module junopl.projectgenerator;

namespace JunoPL {
/**
True when `name` is a valid Juno identifier, so it can be used both as the
program name in main.juno and as a directory name.
 */
export bool isValidProjectName(const std::string &name);

/**
Generates `<parentDir>/<projectName>/main.juno` and `config.json` from the
templates embedded into the executable. Every `{{project}}` placeholder is
replaced with the project name. An existing main.juno or config.json is never
overwritten. Returns the generated project directory, or an error message.
 */
export std::expected<std::filesystem::path, std::string>
generateProject(const std::filesystem::path &parentDir,
                const std::string &projectName);
} // namespace JunoPL

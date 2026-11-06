module;
#include <filesystem>
#include <memory>
#include <ostream>
#include <string>
#include <unordered_set>
#include <vector>
export module junopl.moduleloader;

import junopl.parser.nodes;

export namespace JunoPL {
/**
Loads an entry module and everything it imports into a single AST.

Each module is parsed exactly once and spliced into one root, post-order and
include-once: imports are resolved depth-first so a module's declarations
land after everything it imports (top-level `var`/`list` initializers execute
in body order), a module already in `mMerged` is skipped, and an import found
in `mChain` is a cycle. ImportNode entries never survive into the returned
root, so no import reaches the semantic analyzer.

Diagnostics are rendered through junopl.diagnostics onto the stream passed to
load(); the loader itself holds the state of exactly one load.
 */
class ModuleLoader {
  public:
    /**
    Loads `entry` and its transitive imports. Returns the merged AST, or
    nullptr with the failure diagnostics already written to `errors`.
     */
    std::unique_ptr<RootNode> load(const std::filesystem::path &entry,
                                   std::ostream &errors);

  private:
    bool processModule(const std::filesystem::path &canonicalPath,
                       std::ostream &errors);

    std::unique_ptr<RootNode> mRoot;                 // built by the load
    std::unordered_set<std::string> mMerged;         // canonical path -> merged
    std::vector<std::filesystem::path> mChain;       // modules in progress
};
} // namespace JunoPL

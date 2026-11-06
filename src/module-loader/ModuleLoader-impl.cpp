module;
#include <algorithm>
#include <filesystem>
#include <memory>
#include <ostream>
#include <string>
#include <system_error>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>
module junopl.moduleloader;

import junopl.common.location;
import junopl.diagnostics;
import junopl.filehandler;
import junopl.lexer;
import junopl.parser;
import junopl.parser.nodes;
import junopl.semantic.diagnostic;

namespace fs = std::filesystem;

namespace {
// Absolute, normalized spelling of `p`, used as the identity of a module for
// the cycle and already-merged checks: "util.juno", "./util.juno" and
// "../lib/util.juno" must all map to the same key.
fs::path normalizePath(const fs::path &p) {
    std::error_code ec;
    fs::path abs = p.is_absolute() ? p : fs::absolute(p, ec);
    if (ec) {
        abs = p;
    }
    fs::path normalized = fs::weakly_canonical(abs, ec);
    if (ec) {
        return abs.lexically_normal();
    }
    return normalized;
}
} // namespace

namespace JunoPL {
std::unique_ptr<RootNode>
ModuleLoader::load(const fs::path &entry, std::ostream &errors) {
    if (!processModule(normalizePath(entry), errors)) {
        return nullptr;
    }
    return std::move(mRoot);
}

bool ModuleLoader::processModule(const fs::path &canonicalPath,
                                 std::ostream &errors) {
    mChain.push_back(canonicalPath);
    const bool ok = [&]() -> bool {
        auto fileResult = FileHandler::open(canonicalPath);
        if (!fileResult.has_value()) {
            printFileError(errors, canonicalPath, fileResult.error());
            return false;
        }
        auto &fileHandler = *fileResult;

        // Read source once so every diagnostic can show a source snippet.
        auto linesResult = FileHandler::readLines(canonicalPath);
        if (!linesResult.has_value()) {
            printFileError(errors, canonicalPath, linesResult.error());
            return false;
        }
        const SourceLines sourceLines = *linesResult;

        Lexer lexer;
        auto lexOutput = lexer.tokenize(fileHandler);
        if (!lexOutput.errors.empty()) {
            errors << "Lexer errors:\n";
            for (const auto &err : lexOutput.errors) {
                SourceLocation loc{err.line(), err.column(),
                                   err.column() + 1};
                printDiagnostic(errors, canonicalPath, sourceLines,
                                Diagnostic::Severity::Error, loc,
                                err.message());
            }
            return false;
        }

        Parser parser;
        auto output = parser.parse(lexOutput.tokens);
        if (!output.errors.empty()) {
            errors << "Parser errors:\n";
            for (const auto &err : output.errors) {
                printDiagnostic(errors, canonicalPath, sourceLines,
                                Diagnostic::Severity::Error, err.location(),
                                err.message());
            }
            return false;
        }
        if (!output.root) {
            errors << "error: no AST produced\n";
            return false;
        }

        // Collect the imports before merging: merge() drops ImportNode
        // entries, and each import path is relative to this file.
        std::vector<ImportNode> imports;
        for (auto &node : output.root->body) {
            if (const auto *importNode = std::get_if<ImportNode>(&node)) {
                imports.push_back(*importNode);
            }
        }

        if (!mRoot) {
            // Only the entry file reaches this branch: the root carries its
            // program name and location, its declarations arrive by merge.
            mRoot = std::make_unique<RootNode>();
            mRoot->programName = std::move(output.root->programName);
            mRoot->loc = output.root->loc;
        }

        const fs::path baseDir = canonicalPath.parent_path();
        for (const auto &importNode : imports) {
            // Prefer the path relative to the importing file, then the
            // cwd-relative spelling the user may have written.
            fs::path candidate = baseDir / importNode.path;
            if (!FileHandler::exists(candidate)) {
                std::error_code ec;
                fs::path cwdRelative = fs::absolute(importNode.path, ec);
                if (!ec && FileHandler::exists(cwdRelative)) {
                    candidate = cwdRelative;
                }
            }
            const fs::path importPath = normalizePath(candidate);

            if (std::find(mChain.begin(), mChain.end(), importPath) !=
                mChain.end()) {
                std::string cycle;
                for (const auto &step : mChain) {
                    cycle += step.string() + " -> ";
                }
                cycle += importPath.string();
                printDiagnostic(errors, canonicalPath, sourceLines,
                                Diagnostic::Severity::Error, importNode.loc,
                                "import cycle: " + cycle);
                return false;
            }
            if (mMerged.contains(importPath.string())) {
                continue; // already spliced into the root (include-once)
            }
            if (!processModule(importPath, errors)) {
                return false;
            }
        }

        mRoot->merge(std::move(*output.root)); // drops ImportNodes
        mMerged.insert(canonicalPath.string());
        return true;
    }();
    mChain.pop_back();
    return ok;
}
} // namespace JunoPL

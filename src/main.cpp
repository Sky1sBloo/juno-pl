#include <algorithm>
#include <expected>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <system_error>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

import junopl.common.location;
import junopl.config;
import junopl.filehandler;
import junopl.lexer;
import junopl.parser;
import junopl.parser.nodes;
import junopl.semantic;
import junopl.semantic.diagnostic;
import junopl.compiler;

namespace fs = std::filesystem;

namespace {

using SourceLines = std::vector<std::string>;

SourceLines readSourceLines(const fs::path &path) {
    SourceLines lines;
    std::ifstream in(path);
    std::string line;
    while (std::getline(in, line)) {
        lines.push_back(line);
    }
    return lines;
}

std::string severityLabel(JunoPL::Diagnostic::Severity severity) {
    switch (severity) {
    case JunoPL::Diagnostic::Severity::Error:
        return "error";
    case JunoPL::Diagnostic::Severity::Warning:
        return "warning";
    case JunoPL::Diagnostic::Severity::Note:
        return "note";
    }
    return "error";
}

// Print a single diagnostic with file:line:col, a source snippet, and a caret
// pointing at the offending columns.
void printDiagnostic(std::ostream &out, const fs::path &path,
                     const SourceLines &sourceLines,
                     JunoPL::Diagnostic::Severity severity,
                     const JunoPL::SourceLocation &loc,
                     const std::string &message) {
    out << severityLabel(severity) << ": " << message << "\n";
    if (!loc.isValid()) {
        return;
    }
    // Source locations are 0-based; display 1-based for users.
    int displayLine = loc.line + 1;
    int displayCol = loc.colStart + 1;
    out << "  --> " << path.string() << ":" << displayLine << ":" << displayCol
        << "\n";

    if (loc.line < 0 || loc.line >= (int)sourceLines.size()) {
        return;
    }
    const std::string &src = sourceLines[loc.line];
    int col = loc.colStart < 0 ? 0 : loc.colStart;
    if (col > (int)src.size()) {
        col = (int)src.size();
    }
    int width = loc.colEnd - loc.colStart;
    if (width < 1) {
        width = 1;
    }
    int end = col + width;
    if (end > (int)src.size()) {
        end = (int)src.size();
    }

    out << "   | " << src << "\n";
    out << "   | ";
    for (int i = 0; i < col; ++i) {
        out << " ";
    }
    for (int i = col; i < end; ++i) {
        out << "^";
    }
    out << "\n";
}

void printFileError(std::ostream &out, const fs::path &path,
                    const JunoPL::FileHandlerError &error) {
    out << "error: cannot access file '" << path.string() << "': "
        << error.message();
    if (!error.osMessage.empty()) {
        out << " (" << error.osMessage << ")";
    }
    out << "\n";
}

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

/**
Reads, lexes and parses one module and splices its declarations into `root`,
recursing into its imports *first* so a module's own declarations are merged
only after everything it imports (post-order). Top-level `var`/`list`
initializers execute in body order, so this guarantees a module's
dependencies are initialized before the declarations that read them.

chain holds the modules currently being processed — an import found in it
is a real cycle, reported with the chain that closes it. `merged` holds the
canonical paths whose declarations are already in `root`, which makes a
diamond import (two modules importing the same file) splice that file once.

The first module processed — the entry file — creates the root and keeps its
programName/loc; its body goes through the same merge path as every other
module, which also drops its ImportNode entries so no import ever reaches the
semantic analyzer. Returns false on failure with the diagnostic printed.
 */
bool processModule(const fs::path &canonicalPath,
                   std::unique_ptr<JunoPL::RootNode> &root,
                   std::unordered_set<std::string> &merged,
                   std::vector<fs::path> &chain) {
    chain.push_back(canonicalPath);
    const bool ok = [&]() -> bool {
        auto fileResult = JunoPL::FileHandler::open(canonicalPath);
        if (!fileResult.has_value()) {
            printFileError(std::cerr, canonicalPath, fileResult.error());
            return false;
        }
        auto &fileHandler = *fileResult;

        // Read source once so every diagnostic can show a source snippet.
        const SourceLines sourceLines = readSourceLines(canonicalPath);

        JunoPL::Lexer lexer;
        auto lexOutput = lexer.tokenize(fileHandler);
        if (!lexOutput.errors.empty()) {
            std::cerr << "Lexer errors:\n";
            for (const auto &err : lexOutput.errors) {
                JunoPL::SourceLocation loc{err.line(), err.column(),
                                           err.column() + 1};
                printDiagnostic(std::cerr, canonicalPath, sourceLines,
                                JunoPL::Diagnostic::Severity::Error, loc,
                                err.message());
            }
            return false;
        }

        JunoPL::Parser parser;
        auto output = parser.parse(lexOutput.tokens);
        if (!output.errors.empty()) {
            std::cerr << "Parser errors:\n";
            for (const auto &err : output.errors) {
                printDiagnostic(std::cerr, canonicalPath, sourceLines,
                                JunoPL::Diagnostic::Severity::Error,
                                err.location(), err.message());
            }
            return false;
        }
        if (!output.root) {
            std::cerr << "error: no AST produced\n";
            return false;
        }

        // Collect the imports before merging: merge() drops ImportNode
        // entries, and each import path is relative to this file.
        std::vector<JunoPL::ImportNode> imports;
        for (auto &node : output.root->body) {
            if (const auto *importNode =
                    std::get_if<JunoPL::ImportNode>(&node)) {
                imports.push_back(*importNode);
            }
        }

        if (!root) {
            // Only the entry file reaches this branch: the root carries its
            // program name and location, its declarations arrive by merge.
            root = std::make_unique<JunoPL::RootNode>();
            root->programName = std::move(output.root->programName);
            root->loc = output.root->loc;
        }

        const fs::path baseDir = canonicalPath.parent_path();
        for (const auto &importNode : imports) {
            // Prefer the path relative to the importing file, then the
            // cwd-relative spelling the user may have written.
            fs::path candidate = baseDir / importNode.path;
            std::error_code ec;
            if (!fs::exists(candidate, ec)) {
                std::error_code ec2;
                fs::path cwdRelative = fs::absolute(importNode.path, ec2);
                if (!ec2 && fs::exists(cwdRelative, ec2)) {
                    candidate = cwdRelative;
                }
            }
            const fs::path importPath = normalizePath(candidate);

            if (std::find(chain.begin(), chain.end(), importPath) !=
                chain.end()) {
                std::string cycle;
                for (const auto &step : chain) {
                    cycle += step.string() + " -> ";
                }
                cycle += importPath.string();
                printDiagnostic(std::cerr, canonicalPath, sourceLines,
                                JunoPL::Diagnostic::Severity::Error,
                                importNode.loc, "import cycle: " + cycle);
                return false;
            }
            if (merged.contains(importPath.string())) {
                continue; // already spliced into the root (include-once)
            }
            if (!processModule(importPath, root, merged, chain)) {
                return false;
            }
        }

        root->merge(std::move(*output.root)); // drops ImportNodes
        merged.insert(canonicalPath.string());
        return true;
    }();
    chain.pop_back();
    return ok;
}

struct Options {
    fs::path source;         // -c: the file to compile
    bool searchMain = false; // -P: look for main.juno instead
    fs::path outDir;         // -d: output directory
    bool help = false;
};

void printUsage(std::ostream &out) {
    out << "Usage: juno-pl -c <source.juno> [-d <output-dir>]\n"
           "       juno-pl -P [-d <output-dir>]\n"
           "\n"
           "  -c <file>    compile the given source file\n"
           "  -P           search for main.juno in the current directory and\n"
           "               its parents; -c is used as given when both appear\n"
           "  -d <dir>     output directory; defaults to \"outputDir\" from\n"
           "               config.json, searched the same way as main.juno\n"
           "  -h, --help   show this help\n"
           "\n"
           "Without -d and without a config.json the XML is written next to\n"
           "the source file.\n";
}

// Reads the command line. Returns nothing and fills `error` when the
// arguments do not make sense.
std::optional<Options> parseOptions(int argc, char *argv[],
                                    std::string &error) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            options.help = true;
        } else if (arg == "-c" || arg == "-d") {
            if (i + 1 >= argc) {
                error = arg + " expects an argument";
                return std::nullopt;
            }
            const fs::path value(argv[++i]);
            if (arg == "-c") {
                options.source = value;
            } else {
                options.outDir = value;
            }
        } else if (arg == "-P") {
            options.searchMain = true;
        } else {
            error = "unknown argument: " + arg;
            return std::nullopt;
        }
    }

    if (!options.help && options.source.empty() && !options.searchMain) {
        error = "no input given, pass -c <file> or -P";
        return std::nullopt;
    }
    return options;
}
}

int main(int argc, char *argv[]) {
    std::string argumentError;
    auto options = parseOptions(argc, argv, argumentError);
    if (!options) {
        std::cerr << "error: " << argumentError << "\n";
        printUsage(std::cerr);
        return 1;
    }
    if (options->help) {
        printUsage(std::cout);
        return 0;
    }

    const fs::path cwd = fs::current_path();

    // The config supplies the default output directory, so it is read
    // even when -d overrides it: a broken config should not go unnoticed
    auto configResult = JunoPL::loadConfig(cwd);
    if (!configResult) {
        std::cerr << "error: " << configResult.error() << "\n";
        return 1;
    }
    const JunoPL::Config &config = *configResult;

    fs::path inputPath;
    if (!options->source.empty()) {
        inputPath = options->source;
    } else {
        const auto mainJuno = JunoPL::findFileUpwards(cwd, "main.juno");
        if (!mainJuno) {
            std::cerr << "error: no main.juno in " << cwd
                      << " or any parent directory; pass -c <file> to name "
                         "the source\n";
            return 1;
        }
        inputPath = *mainJuno;
    }
    const fs::path outputDir =
        JunoPL::outputDirectory(options->outDir, config, inputPath);
    const fs::path outputPath =
        outputDir / (inputPath.stem().string() + ".xml");

    if (!outputDir.empty()) {
        std::error_code ec;
        fs::create_directories(outputDir, ec);
        if (ec) {
            std::cerr << "error: cannot create output directory '" << outputDir
                      << "': " << ec.message() << "\n";
            return 1;
        }
    }

    // Load the entry file and everything it imports: each module is parsed
    // exactly once and spliced into one root AST (post-order, include-once).
    std::unique_ptr<JunoPL::RootNode> root;
    std::unordered_set<std::string> mergedPaths; // canonical path -> merged
    std::vector<fs::path> importChain;           // modules being processed
    if (!processModule(normalizePath(inputPath), root, mergedPaths,
                       importChain)) {
        return 1;
    }

    // Snippet source for semantic diagnostics. SourceLocation does not carry
    // a file identity yet, so a diagnostic from an imported module cannot be
    // matched to its file — an empty line list prints the file:line:col
    // header only. This is where the per-file source table slots in.
    const SourceLines sourceLines;

    JunoPL::SemanticAnalyzer analyzer;
    auto diagnostics = analyzer.analyze(*root);
    if (!diagnostics.empty()) {
        std::cerr << "Semantic errors:\n";
        for (const auto &diag : diagnostics) {
            printDiagnostic(std::cerr, inputPath, sourceLines, diag.severity,
                            diag.loc, diag.message);
        }
        return 1;
    }

    JunoPL::Compiler compiler;
    compiler.getVariables(*root);
    std::string xml = compiler.compileAST(*root);

    auto writeResult = compiler.writeToFile(outputPath, xml);
    if (!writeResult.has_value()) {
        printFileError(std::cerr, outputPath, writeResult.error());
        return 1;
    }

    std::cout << "Wrote " << outputPath << "\n";
    return 0;
}

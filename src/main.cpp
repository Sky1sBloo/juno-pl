#include <expected>
#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <system_error>
#include <vector>

import junopl.compiler;
import junopl.config;
import junopl.diagnostics;
import junopl.filehandler;
import junopl.moduleloader;
import junopl.parser.nodes;
import junopl.semantic;
import junopl.semantic.diagnostic;

namespace fs = std::filesystem;

namespace {

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
           "the source file. The file is named after the program declaration;\n"
           "config.json's \"outputFile\" overrides that name.\n";
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
} // namespace

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

    if (!outputDir.empty()) {
        std::error_code ec;
        fs::create_directories(outputDir, ec);
        if (ec) {
            std::cerr << "error: cannot create output directory '" << outputDir
                      << "': " << ec.message() << "\n";
            return 1;
        }
    }

    // Load the entry file and everything it imports into one root AST.
    JunoPL::ModuleLoader loader;
    auto root = loader.load(inputPath, std::cerr);
    if (!root) {
        return 1;
    }

    const fs::path outputPath =
        outputDir / JunoPL::outputFileName(config, root->programName);

    // Snippet source for semantic diagnostics. SourceLocation does not carry
    // a file identity yet, so a diagnostic from an imported module cannot be
    // matched to its file — an empty line list prints the file:line:col
    // header only. This is where the per-file source table slots in.
    const JunoPL::SourceLines sourceLines;

    JunoPL::SemanticAnalyzer analyzer;
    auto diagnostics = analyzer.analyze(*root);
    if (!diagnostics.empty()) {
        std::cerr << "Semantic errors:\n";
        for (const auto &diag : diagnostics) {
            JunoPL::printDiagnostic(std::cerr, inputPath, sourceLines,
                                    diag.severity, diag.loc, diag.message);
        }
        return 1;
    }

    JunoPL::Compiler compiler;
    compiler.getVariables(*root);
    auto compileResult = compiler.compileAST(*root);
    if (!compileResult) {
        std::cerr << "error: " << compileResult.error() << "\n";
        return 1;
    }
    std::string xml = *compileResult;

    auto writeResult = compiler.writeToFile(outputPath, xml);
    if (!writeResult.has_value()) {
        JunoPL::printFileError(std::cerr, outputPath, writeResult.error());
        return 1;
    }

    std::cout << "Wrote " << outputPath << "\n";
    return 0;
}

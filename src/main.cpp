#include <expected>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <system_error>
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
    out << "  --> " << path << ":" << displayLine << ":" << displayCol << "\n";

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
    out << "error: cannot access file '" << path << "': " << error.message();
    if (!error.osMessage.empty()) {
        out << " (" << error.osMessage << ")";
    }
    out << "\n";
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

    // Open input file
    auto fileResult = JunoPL::FileHandler::open(inputPath);
    if (!fileResult.has_value()) {
        printFileError(std::cerr, inputPath, fileResult.error());
        return 1;
    }
    auto &fileHandler = *fileResult;

    // Read source once so every diagnostic can show a source snippet.
    auto sourceLines = readSourceLines(inputPath);

    // Lex
    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileHandler);
    if (!lexOutput.errors.empty()) {
        std::cerr << "Lexer errors:\n";
        for (const auto &err : lexOutput.errors) {
            JunoPL::SourceLocation loc{err.line(), err.column(),
                                       err.column() + 1};
            printDiagnostic(std::cerr, inputPath, sourceLines,
                            JunoPL::Diagnostic::Severity::Error, loc,
                            err.message());
        }
        return 1;
    }

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);
    if (!parseOutput.errors.empty()) {
        std::cerr << "Parser errors:\n";
        for (const auto &err : parseOutput.errors) {
            printDiagnostic(std::cerr, inputPath, sourceLines,
                            JunoPL::Diagnostic::Severity::Error, err.location(),
                            err.message());
        }
        return 1;
    }
    if (!parseOutput.root) {
        std::cerr << "error: no AST produced\n";
        return 1;
    }

    JunoPL::SemanticAnalyzer analyzer;
    auto diagnostics = analyzer.analyze(*parseOutput.root);
    if (!diagnostics.empty()) {
        std::cerr << "Semantic errors:\n";
        for (const auto &diag : diagnostics) {
            printDiagnostic(std::cerr, inputPath, sourceLines, diag.severity,
                            diag.loc, diag.message);
        }
        return 1;
    }

    JunoPL::Compiler compiler;
    compiler.getVariables(*parseOutput.root);
    std::string xml = compiler.compileAST(*parseOutput.root);

    auto writeResult = compiler.writeToFile(outputPath, xml);
    if (!writeResult.has_value()) {
        printFileError(std::cerr, outputPath, writeResult.error());
        return 1;
    }

    std::cout << "Wrote " << outputPath << "\n";
    return 0;
}

#include <expected>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

import junopl.common.location;
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

} // namespace

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: juno-pl <input.juno> [output.xml]\n";
        return 1;
    }

    fs::path inputPath(argv[1]);
    fs::path outputPath =
        (argc >= 3) ? fs::path(argv[2]) : fs::path(inputPath).replace_extension(".xml");

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

    // Parse
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

    // Semantic analysis
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

    // Compile to XML
    JunoPL::Compiler compiler;
    compiler.getVariables(*parseOutput.root);
    std::string xml = compiler.compileAST(*parseOutput.root);

    // Write output
    auto writeResult = compiler.writeToFile(outputPath, xml);
    if (!writeResult.has_value()) {
        printFileError(std::cerr, outputPath, writeResult.error());
        return 1;
    }

    std::cout << "Wrote " << outputPath << "\n";
    return 0;
}

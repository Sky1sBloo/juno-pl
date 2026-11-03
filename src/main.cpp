#include <iostream>
#include <string>
#include <filesystem>
#include <expected>

import junopl.lexer;
import junopl.parser;
import junopl.parser.nodes;
import junopl.semantic;
import junopl.compiler;
import junopl.filehandler;

namespace fs = std::filesystem;

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
        std::cerr << "Error: cannot open input file '" << inputPath << "'\n";
        return 1;
    }
    auto &fileHandler = *fileResult;

    // Lex
    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileHandler);
    if (!lexOutput.errors.empty()) {
        std::cerr << "Lexer errors:\n";
        for (const auto &err : lexOutput.errors) {
            std::cerr << "  line " << err.line() << ":" << err.column()
                      << ": " << err.message() << "\n";
        }
        return 1;
    }

    // Parse
    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);
    if (!parseOutput.errors.empty()) {
        std::cerr << "Parser errors:\n";
        for (const auto &err : parseOutput.errors) {
            std::cerr << "  " << err.message() << "\n";
        }
        return 1;
    }
    if (!parseOutput.root) {
        std::cerr << "Error: no AST produced\n";
        return 1;
    }

    // Semantic analysis
    JunoPL::SemanticAnalyzer analyzer;
    auto diagnostics = analyzer.analyze(*parseOutput.root);
    if (!diagnostics.empty()) {
        std::cerr << "Semantic errors:\n";
        for (const auto &diag : diagnostics) {
            std::cerr << "  " << diag.message << "\n";
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
        std::cerr << "Error: cannot write output file '" << outputPath << "'\n";
        return 1;
    }

    std::cout << "Wrote " << outputPath << "\n";
    return 0;
}

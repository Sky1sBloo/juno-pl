module;
#include <filesystem>
#include <ostream>
#include <string>
#include <vector>
export module junopl.diagnostics;

import junopl.common.location;
import junopl.ifilehandler;
import junopl.semantic.diagnostic;

export namespace JunoPL {
using SourceLines = std::vector<std::string>;

std::string severityLabel(Diagnostic::Severity severity);

/**
Print a single diagnostic with file:line:col, a source snippet, and a caret
pointing at the offending columns. An empty `sourceLines`, an invalid
location, or a line outside the buffer prints the header only.
 */
void printDiagnostic(std::ostream &out, const std::filesystem::path &path,
                     const SourceLines &sourceLines,
                     Diagnostic::Severity severity, const SourceLocation &loc,
                     const std::string &message);

void printFileError(std::ostream &out, const std::filesystem::path &path,
                    const FileHandlerError &error);
} // namespace JunoPL

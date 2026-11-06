module;
#include <filesystem>
#include <ostream>
#include <string>
#include <vector>
module junopl.diagnostics;

import junopl.common.location;
import junopl.ifilehandler;
import junopl.semantic.diagnostic;

namespace JunoPL {
std::string severityLabel(Diagnostic::Severity severity) {
    switch (severity) {
    case Diagnostic::Severity::Error:
        return "error";
    case Diagnostic::Severity::Warning:
        return "warning";
    case Diagnostic::Severity::Note:
        return "note";
    }
    return "error";
}

void printDiagnostic(std::ostream &out, const std::filesystem::path &path,
                     const SourceLines &sourceLines,
                     Diagnostic::Severity severity, const SourceLocation &loc,
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

void printFileError(std::ostream &out, const std::filesystem::path &path,
                    const FileHandlerError &error) {
    out << "error: cannot access file '" << path.string() << "': "
        << error.message();
    if (!error.osMessage.empty()) {
        out << " (" << error.osMessage << ")";
    }
    out << "\n";
}
} // namespace JunoPL

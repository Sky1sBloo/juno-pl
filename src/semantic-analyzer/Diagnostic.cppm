module;
#include <string>
export module junopl.semantic.diagnostic;
import junopl.common.location;

namespace JunoPL {
export struct Diagnostic {
    enum class Severity { Error, Warning, Note };
    std::string message;
    SourceLocation loc;
    Severity severity = Severity::Error;
};
}

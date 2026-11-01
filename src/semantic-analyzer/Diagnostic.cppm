module;
#include <string>
export module junopl.semantic.diagnostic;
import junopl.common.location;

namespace JunoPL {
export struct Diagnostic {
    std::string message;
    SourceLocation loc;
};
}
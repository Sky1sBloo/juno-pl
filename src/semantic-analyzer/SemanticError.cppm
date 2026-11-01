module;
#include <string>
export module junopl.semantic.error;

namespace JunoPL {
  export struct SemanticError {
    std::string message;
  };
}
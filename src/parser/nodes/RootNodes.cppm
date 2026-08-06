module;
#include <memory>
#include <string>
#include <variant>
#include <vector>
export module junopl.parser.nodes;
import junopl.parser.nodes.statements;

namespace JunoPL {
export struct ImportNode {
    std::string path;
};

export struct EventNode {
    std::string identifier;
    std::vector<std::string> params;
    std::unique_ptr<Body> body;
};

export struct FunctionNode {
    std::string identifier;
    std::vector<std::string> params;
    std::unique_ptr<Body> body;
};

using RootNodes = std::variant<ImportNode, EventNode, FunctionNode, VarDeclaration, ListDeclaration>;
export struct RootNode {
    std::string programName;
    std::vector<RootNodes> body;
};

}
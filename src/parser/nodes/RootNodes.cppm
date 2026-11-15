module;
#include <string>
#include <utility>
#include <variant>
#include <vector>
export module junopl.parser.nodes;
import junopl.common.location;
import junopl.parser.nodes.statements;
import junopl.parser.nodes.expressions;

namespace JunoPL {
export struct ImportNode {
    std::string path;
    SourceLocation loc;
};

export struct EventNode {
    std::string identifier;
    std::vector<std::string> params;
    Body body;
    SourceLocation loc;
};

// delimeter is added on the compiler not here
export struct ReceiveSocketNode {
    std::string port;
    std::vector<std::string> params;
    Body body;
    SourceLocation loc;
};

export struct FunctionNode {
    std::string identifier;
    std::vector<std::string> params;
    Body body;
    SourceLocation loc;
};

using RootNodes =
    std::variant<ImportNode, EventNode, ReceiveSocketNode, FunctionNode,
                 VarDeclaration, CustomExpression, ListDeclaration>;
export struct RootNode {
    std::string programName;
    std::vector<RootNodes> body;
    SourceLocation loc;

    /**
    Splices other rootnodes to this root node
     */
    void merge(RootNode &&other) {
        for (RootNodes &node : other.body) {
            if (std::holds_alternative<ImportNode>(node)) {
                continue;
            }
            body.push_back(std::move(node));
        }
        other.body.clear();
    }
};

}

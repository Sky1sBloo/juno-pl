module;
#include <expected>
#include <span>
#include <vector>
export module junopl.parser;
import junopl.lexer.tokens;
import junopl.parser.nodes;
import junopl.parser.error;

namespace JunoPL {
export class Parser {
  public:
    std::expected<RootNode, ParserError>
    parse(const std::vector<Token> &tokens);

  private:
    std::expected<RootNode, ParserError>
    handleProgramName(std::span<Token> tokens);
};
}
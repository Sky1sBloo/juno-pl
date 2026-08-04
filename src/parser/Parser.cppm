module;
#include <vector>
export module junopl.parser;
import junopl.lexer.tokens;
import junopl.parser.nodes;

namespace JunoPL {
export class Parser {
  public:
    RootNode parse(const std::vector<Token> &tokens);

  private:
};
}
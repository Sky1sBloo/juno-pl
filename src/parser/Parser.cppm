module;
#include <expected>
#include <memory>
#include <string>
#include <vector>
export module junopl.parser;
import junopl.lexer.tokens;
import junopl.parser.nodes;
import junopl.parser.error;

namespace JunoPL {
export class Parser {
  public:
    struct Output {
        std::unique_ptr<RootNode> root;
        std::vector<ParserError> errors;
    };

  public:
    Output parse(TokenList &tokens);

  private:
    std::expected<std::string, ParserError>
    handleProgramName(TokenList &tokens);

    std::expected<ImportNode, ParserError> handleImport(TokenList &tokens);

    std::expected<Token, ParserError> expectToken(TokenList &tokens,
                                                  TokenType expected);
};
}
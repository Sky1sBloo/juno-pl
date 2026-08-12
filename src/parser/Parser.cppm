module;
#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <vector>
export module junopl.parser;
import junopl.lexer.tokens;
import junopl.parser.nodes;
import junopl.parser.nodes.statements;
import junopl.parser.nodes.expressions;
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

    // Individual parse methods (public for testing and free function wrappers)
    std::optional<FunctionNode> parseFunction(TokenList &tokens);
    std::optional<CustomExpression> parseCustomExpression(TokenList &tokens);
    std::optional<EventNode> parseEvent(TokenList &tokens);
    std::optional<VarDeclaration> parseVarDeclaration(TokenList &tokens);
    std::optional<ListDeclaration> parseListDeclaration(TokenList &tokens);
    std::optional<PerformInstruction> parsePerformInstruction(TokenList &tokens);
    std::optional<ConditionalStatement> parseConditionalStatement(TokenList &tokens);
    std::optional<RepeatLoop> parseRepeatLoop(TokenList &tokens);
    std::optional<WhileLoop> parseWhileLoop(TokenList &tokens);
    std::optional<ForLoop> parseForLoop(TokenList &tokens);
    std::optional<EmitEvent> parseEmitEvent(TokenList &tokens);
    std::optional<std::vector<std::string>> parseParam(TokenList &tokens);
    std::optional<std::vector<ExpressionHandle>> parseParamExpr(TokenList &tokens);
    std::optional<Body> parseBody(TokenList &tokens);
    std::optional<ExpressionHandle> parseExpression(TokenList &tokens);

  private:
    TokenList *mTokens{nullptr};
    std::vector<ParserError> mErrors;

    std::optional<Token> expectToken(TokenType expectedType, bool advance = true);
    std::optional<Token> expectToken(std::initializer_list<TokenType> expectedTypes, bool advance = true);
    void recoverTo(TokenType stopType);
    void pushError(ParserError error);

    std::optional<std::string> handleProgramName();
    std::optional<ImportNode> handleImport();

    // Internal recursive helpers (use mTokens directly)
    std::optional<ExpressionHandle> parsePrimaryExpression();
    std::optional<ExpressionHandle> parseUnaryExpression();
    std::optional<ExpressionHandle> parseBinaryExpression(int minimumPrecedence = 0);
    std::optional<ExpressionHandle> parseTernaryExpression();
};
}

module;
#include <optional>
#include <string>
export module junopl.lexer.statehandler;
import junopl.lexer.error;
import junopl.lexer.tokens;

namespace JunoPL {
export class LexerStateHandler {
  public:
    struct Result {
        enum class Action { CONTINUE, SAVE_TOKEN, SAVE_REPLAY, ERROR };
        Action action;
        TokenType type = TokenType::UNKNOWN;
        std::optional<LexerError> error;

        static Result Continue();
        static Result Save(TokenType type);
        static Result SaveReplay(TokenType type);
        static Result Error(const LexerError &error);

      private:
        Result(Action action, TokenType type);
        Result(LexerError error);
    };

  public:
    LexerStateHandler();

    void reset();

    /**
    Handles specific character c and moves to a specific state.
    @returns LexerError if invalid character based on state
     */
    Result handle(char c, int line, int col);

  private:
    enum class States { START, IDENT, NUMBER, UNKNOWN };
    States mState;
    int mLine;
    int mCol;

    Result handleStartState(char c);
    Result handleIdentState(char c);
    Result handleNumberState(char c);
    Result handleUnknownState(char c);
    Result createErrorResult(const std::string &message);
};
}
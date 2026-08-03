module;
#include <optional>
export module junopl.lexer.statehandler;
import junopl.lexer.error;
import junopl.lexer.tokens;

namespace JunoPL {
export class LexerStateHandler {
  public:
    struct Result {
        enum class Action {
            CONTINUE,    // Add character to lexeme then continue
            IGNORE,      // Ignore character then continue
            REPLAY,      // Handle the character again (usually for handling in
                         // different state)
            SAVE_TOKEN,  // Add to lexeme and save token
            SAVE_REPLAY, // Save token (without this character) then replay this
                         // character
            SAVE_IGNORE, // Save token then ignore this character
            INFER_TOKEN, // Similar to SAVE_TOKEN but infers token type
            INFER_TOKEN_REPLAY, // Similar to SAVE_REPLAY but infers token type
            ERROR
        };
        Action action;
        TokenType type = TokenType::UNKNOWN;
        std::optional<LexerError> error;

        static Result Continue();
        static Result Ignore();
        static Result Replay();
        static Result Save(TokenType type);
        static Result SaveReplay(TokenType type);
        static Result SaveIgnore(TokenType type);
        static Result InferToken();
        static Result InferTokenReplay();
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

    /**
    Call this when we reach EOF.
     */
    Result handleEOF();

  private:
    enum class States {
        START,
        IDENT,
        NUMBER,
        DECIMAL,
        STRING,
        STRING_ESCAPE,
        OPERATION,
        EXPECT_EQ,
        UNKNOWN
    };
    States mState;
    int mLine;
    int mCol;
    bool isCharEOF; // used for eof cases
    Result createErrorResult(char c);

    Result handleStartState(char c);
    Result handleIdentState(char c);
    Result handleDecimalState(char c);
    Result handleStringState(char c);
    Result handleStringEscapeState(char c);
    Result handleOperationState(char c);
    Result handleExpectEqState(char c);
    Result handleNumberState(char c);
    Result handleUnknownState(char c);

    bool isOpSymbol(char c);
};
}
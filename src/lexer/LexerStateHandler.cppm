export module junopl.lexer.statehandler;

namespace JunoPL {
class LexerStateHandler {
  public:
    enum class State { START };

    void reset();

  private:
    State mState;
};
}
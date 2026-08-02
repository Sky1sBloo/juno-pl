module junopl.lexer.statehandler;

namespace JunoPL {
    void LexerStateHandler::reset() {
        mState = State::START;
    }
}
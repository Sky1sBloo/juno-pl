module;
#include <optional>
module junopl.lexer.statehandler;
import junopl.lexer.tokens;

namespace JunoPL {
LexerStateHandler::Result::Result(Action action, TokenType type)
    : action(action), type(type), error(std::nullopt) {}

LexerStateHandler::Result::Result(LexerError error)
    : action(Action::ERROR), type(TokenType::UNKNOWN), error(std::move(error)) {
}

LexerStateHandler::Result LexerStateHandler::Result::Continue() {
    return Result{Action::CONTINUE, TokenType::UNKNOWN};
}

LexerStateHandler::Result LexerStateHandler::Result::Ignore() {
    return Result{Action::IGNORE, TokenType::UNKNOWN};
}

LexerStateHandler::Result LexerStateHandler::Result::Save(TokenType type) {
    return Result{Action::SAVE_TOKEN, type};
}

LexerStateHandler::Result
LexerStateHandler::Result::SaveReplay(TokenType type) {
    return Result{Action::SAVE_REPLAY, type};
}

LexerStateHandler::Result
LexerStateHandler::Result::InferToken() {
    return Result{Action::INFER_TOKEN, TokenType::UNKNOWN};
}

LexerStateHandler::Result
LexerStateHandler::Result::InferTokenReplay() {
    return Result{Action::INFER_TOKEN_REPLAY, TokenType::UNKNOWN};
}

LexerStateHandler::Result
LexerStateHandler::Result::Error(const LexerError &error) {
    return Result{error};
}
}

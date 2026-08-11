#include <expected>
module junopl.parser.handlers;

namespace JunoPL {
std::expected<EmitEvent, ParserError> emitEvent(TokenList &tokens) {}
}
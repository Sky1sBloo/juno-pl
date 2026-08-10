module;
#include <expected>
#include <memory>
module junopl.parser.handlers;

namespace JunoPL {
std::expected<JunoPL::ExpressionHandle, JunoPL::ParserError>
parseSimpleValueExpression(JunoPL::TokenList &tokens) {
	// TODO: parse richer expression content here.
	auto valueToken = JunoPL::expectToken(
		tokens, {JunoPL::TokenType::NUM, JunoPL::TokenType::STR,
				 JunoPL::TokenType::TRUE, JunoPL::TokenType::FALSE,
				 JunoPL::TokenType::IDENT});
	if (!valueToken) {
		return std::unexpected(valueToken.error());
	}

	auto expression = std::make_unique<JunoPL::Expression>();
	expression->value = JunoPL::Value{valueToken.value().value};
	return expression;
}
}
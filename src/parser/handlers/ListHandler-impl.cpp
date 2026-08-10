module;
#include <expected>
module junopl.parser.handlers;

import junopl.lexer.tokens;
import junopl.parser.error;
import junopl.parser.nodes;
import junopl.parser.nodes.expressions;
import junopl.parser.nodes.statements;

namespace JunoPL {
std::expected<ListDeclaration, ParserError>
parseListDeclaration(TokenList &tokens) {
	if (auto kList = expectToken(tokens, TokenType::K_LIST); !kList) {
		return std::unexpected(kList.error());
	}

	auto ident = expectToken(tokens, TokenType::IDENT);
	if (!ident) {
		return std::unexpected(ident.error());
	}

	ListDeclaration node;
	node.identifier = ident.value().value;

	auto next = expectToken(tokens,
							{TokenType::OP_EQUAL, TokenType::OP_SEMICOLON});
	if (!next) {
		return std::unexpected(next.error());
	}

	if (next.value().type == TokenType::OP_SEMICOLON) {
		return node;
	}

	auto openBracket = expectToken(tokens, TokenType::OP_BRAC_OP);
	if (!openBracket) {
		return std::unexpected(openBracket.error());
	}

	if (tokens.current().type != TokenType::OP_BRAC_CLO) {
		while (true) {
			auto value = parseExpression(tokens);
			if (!value) {
				return std::unexpected(value.error());
			}

			node.values.push_back(std::move(value.value()));

			if (tokens.current().type == TokenType::OP_COMMA) {
				tokens.advance();
				continue;
			}

			if (tokens.current().type == TokenType::OP_BRAC_CLO) {
				break;
			}

			return std::unexpected(ParserError::UnexpectedToken(
				tokens.current(),
				{TokenType::OP_COMMA, TokenType::OP_BRAC_CLO}));
		}
	}

	auto closeBracket = expectToken(tokens, TokenType::OP_BRAC_CLO);
	if (!closeBracket) {
		return std::unexpected(closeBracket.error());
	}

	auto semicolon = expectToken(tokens, TokenType::OP_SEMICOLON);
	if (!semicolon) {
		return std::unexpected(semicolon.error());
	}

	return node;
}
}

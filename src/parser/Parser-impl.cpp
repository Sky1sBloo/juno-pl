module;
#include <expected>
#include <string>
module junopl.parser;
import junopl.lexer.tokens;
import junopl.parser.nodes;
import junopl.parser.error;

namespace JunoPL {
Parser::Output Parser::parse(TokenList &tokens) {
    Output output;
    if (tokens.empty()) {
        output.errors.emplace_back(ParserError{ParserError::Type::EMPTY_TOKENS,
                                               "Parser received empty tokens"});
        return output;
    }

    RootNode root;
    switch (tokens.current().type) {
    case TokenType::K_PROGRAM:
        auto programName = handleProgramName(tokens);
        if (programName.has_value()) {
            root.programName = programName.value();
        } else {
            output.errors.push_back(programName.error());
        }
        break;
    }
    return output;
}

std::expected<std::string, ParserError> handleProgramName(TokenList &tokens) {
    tokens.current();
    if (tokens.current().type != TokenType::K_PROGRAM) {
        return std::unexpected(ParserError::UnexpectedToken(
            tokens.current(), TokenType::K_PROGRAM));
    }
    tokens.advance();
    if (tokens.current().type != TokenType::STR) {
        return std::unexpected(
            ParserError::UnexpectedToken(tokens.current(), TokenType::STR));
    }
    std::string programName = tokens.current().value;
    tokens.advance();
    return programName;
}
}

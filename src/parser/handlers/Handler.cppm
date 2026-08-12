module;
#include <optional>
#include <string>
#include <vector>
export module junopl.parser.handlers;

import junopl.lexer.tokens;
import junopl.parser.error;
import junopl.parser.nodes;
import junopl.parser.nodes.statements;
import junopl.parser.nodes.expressions;
import junopl.parser;

namespace JunoPL {

export std::optional<Body> parseBody(TokenList &tokens) {
    Parser parser;
    return parser.parseBody(tokens);
}

export std::optional<FunctionNode> parseFunction(TokenList &tokens) {
    Parser parser;
    return parser.parseFunction(tokens);
}

export std::optional<EventNode> parseEvent(TokenList &tokens) {
    Parser parser;
    return parser.parseEvent(tokens);
}

export std::optional<VarDeclaration> parseVarDeclaration(TokenList &tokens) {
    Parser parser;
    return parser.parseVarDeclaration(tokens);
}

export std::optional<VarAssignment> parseVarAssignment(TokenList &tokens) {
    Parser parser;
    return parser.parseVarAssignment(tokens);
}

export std::optional<ListDeclaration> parseListDeclaration(TokenList &tokens) {
    Parser parser;
    return parser.parseListDeclaration(tokens);
}

export std::optional<PerformInstruction>
parsePerformInstruction(TokenList &tokens) {
    Parser parser;
    return parser.parsePerformInstruction(tokens);
}

export std::optional<BreakStatement> parseBreakStatement(TokenList &tokens) {
    Parser parser;
    return parser.parseBreakStatement(tokens);
}

export std::optional<ConditionalStatement>
parseConditionalStatement(TokenList &tokens) {
    Parser parser;
    return parser.parseConditionalStatement(tokens);
}

export std::optional<RepeatLoop> parseRepeatLoop(TokenList &tokens) {
    Parser parser;
    return parser.parseRepeatLoop(tokens);
}

export std::optional<WhileLoop> parseWhileLoop(TokenList &tokens) {
    Parser parser;
    return parser.parseWhileLoop(tokens);
}

export std::optional<ForLoop> parseForLoop(TokenList &tokens) {
    Parser parser;
    return parser.parseForLoop(tokens);
}

export std::optional<EmitEvent> parseEmitEvent(TokenList &tokens) {
    Parser parser;
    return parser.parseEmitEvent(tokens);
}

export std::optional<CustomExpression>
parseCustomExpression(TokenList &tokens) {
    Parser parser;
    return parser.parseCustomExpression(tokens);
}

export std::optional<std::vector<std::string>> parseParam(TokenList &tokens) {
    Parser parser;
    return parser.parseParam(tokens);
}

export std::optional<std::vector<ExpressionHandle>>
parseParamExpr(TokenList &tokens) {
    Parser parser;
    return parser.parseParamExpr(tokens);
}

export std::optional<ExpressionHandle>
parseExpression(TokenList &tokens) {
    Parser parser;
    return parser.parseExpression(tokens);
}
}

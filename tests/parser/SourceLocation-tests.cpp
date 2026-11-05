#include "doctest.h"
#include <string>
#include <variant>

import junopl.lexer;
import junopl.lexer.tokens;
import junopl.parser;
import junopl.parser.nodes;
import junopl.parser.nodes.expressions;
import junopl.parser.nodes.statements;
import junopl.common.location;
import junopl.tests.filehandler;

/**
Node locations are copies of the token that starts the construct. The
lexer numbers lines from 0 and its colStart points at the delimiter
before the token, so these tests compare node locations against the
actual token positions instead of hand-computed columns. Line numbers
are pinned absolutely since they are the values diagnostics rely on.
 */

namespace {
JunoPL::Token findToken(const JunoPL::TokenList &tokens,
                        JunoPL::TokenType type, int occurrence = 0) {
    int seen = 0;
    for (std::size_t i = 0; i < tokens.size(); ++i) {
        const auto &token = tokens.at(i);
        if (token.type != type) {
            continue;
        }
        if (seen == occurrence) {
            return token;
        }
        ++seen;
    }
    FAIL("token type not found in source");
    return JunoPL::Token{JunoPL::TokenType::UNKNOWN};
}

JunoPL::Token findIdent(const JunoPL::TokenList &tokens,
                        const std::string &value, int occurrence = 0) {
    int seen = 0;
    for (std::size_t i = 0; i < tokens.size(); ++i) {
        const auto &token = tokens.at(i);
        if (token.type != JunoPL::TokenType::IDENT || token.value != value) {
            continue;
        }
        if (seen == occurrence) {
            return token;
        }
        ++seen;
    }
    FAIL("identifier not found in source");
    return JunoPL::Token{JunoPL::TokenType::UNKNOWN};
}

void checkLocation(const JunoPL::SourceLocation &loc,
                   const JunoPL::Token &token) {
    CHECK(loc.line == token.line);
    CHECK(loc.colStart == token.colStart);
    CHECK(loc.colEnd == token.colEnd);
}
} // namespace

TEST_CASE("Source Location - root nodes") {
    JunoPL::Tests::FileHandlerTest fileHandler;
    fileHandler.setSourceCode("program test;\n"
                             "import \"lib\";\n"
                             "instr foo() {\n"
                             "}\n"
                             "on update() {\n"
                             "}\n"
                             "var g = 1;\n"
                             "list l = [1];\n"
                             "expr e() g;\n");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileHandler);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &root = *parseOutput.root;
    checkLocation(root.loc, findToken(lexOutput.tokens, JunoPL::TokenType::K_PROGRAM));
    CHECK(root.loc.line == 0);

    REQUIRE(root.body.size() == 6);

    const auto &importNode = std::get<JunoPL::ImportNode>(root.body.at(0));
    checkLocation(importNode.loc,
                  findToken(lexOutput.tokens, JunoPL::TokenType::K_IMPORT));
    CHECK(importNode.loc.line == 1);

    const auto &function = std::get<JunoPL::FunctionNode>(root.body.at(1));
    checkLocation(function.loc,
                  findToken(lexOutput.tokens, JunoPL::TokenType::K_INSTR));
    CHECK(function.loc.line == 2);

    const auto &event = std::get<JunoPL::EventNode>(root.body.at(2));
    checkLocation(event.loc,
                  findToken(lexOutput.tokens, JunoPL::TokenType::K_ON));
    CHECK(event.loc.line == 4);

    const auto &varDecl = std::get<JunoPL::VarDeclaration>(root.body.at(3));
    checkLocation(varDecl.loc,
                  findToken(lexOutput.tokens, JunoPL::TokenType::K_VAR));
    CHECK(varDecl.loc.line == 6);

    const auto &listDecl = std::get<JunoPL::ListDeclaration>(root.body.at(4));
    checkLocation(listDecl.loc,
                  findToken(lexOutput.tokens, JunoPL::TokenType::K_LIST));
    CHECK(listDecl.loc.line == 7);

    const auto &customExpr = std::get<JunoPL::CustomExpression>(root.body.at(5));
    checkLocation(customExpr.loc,
                  findToken(lexOutput.tokens, JunoPL::TokenType::K_EXPR));
    CHECK(customExpr.loc.line == 8);
}

TEST_CASE("Source Location - statements in body") {
    JunoPL::Tests::FileHandlerTest fileHandler;
    fileHandler.setSourceCode("program test;\n"
                             "instr main() {\n"
                             "    var x = 1;\n"
                             "    x = 2;\n"
                               "    do Lists.clear(items);\n"
                              "    do foo(1);\n"
                             "    emit myEvent(2);\n"
                             "}\n");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileHandler);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &function =
        std::get<JunoPL::FunctionNode>(parseOutput.root->body.at(0));
    const auto &tokens = lexOutput.tokens;

    // Body is located at its opening brace
    checkLocation(function.body.loc,
                  findToken(tokens, JunoPL::TokenType::OP_CBRAC_OP, 0));
    CHECK(function.body.loc.line == 1);

    REQUIRE(function.body.body.size() == 5);

    const auto &varDecl = std::get<JunoPL::VarDeclaration>(
        function.body.body.at(0)->statement);
    checkLocation(varDecl.loc, findToken(tokens, JunoPL::TokenType::K_VAR));
    CHECK(varDecl.loc.line == 2);

    const auto &assignment = std::get<JunoPL::VarAssignment>(
        function.body.body.at(1)->statement);
    checkLocation(assignment.loc, findIdent(tokens, "x", 1));
    CHECK(assignment.loc.line == 3);

    const auto &clearInstr = std::get<JunoPL::DoInstruction>(
        function.body.body.at(2)->statement);
    checkLocation(clearInstr.loc,
                  findToken(tokens, JunoPL::TokenType::K_DO, 0));
    CHECK(clearInstr.loc.line == 4);

    const auto &doInstr = std::get<JunoPL::DoInstruction>(
        function.body.body.at(3)->statement);
    checkLocation(doInstr.loc,
                  findToken(tokens, JunoPL::TokenType::K_DO, 1));
    CHECK(doInstr.loc.line == 5);

    const auto &emit =
        std::get<JunoPL::EmitEvent>(function.body.body.at(4)->statement);
    checkLocation(emit.loc, findToken(tokens, JunoPL::TokenType::K_EMIT));
    CHECK(emit.loc.line == 6);
}

TEST_CASE("Source Location - conditionals") {
    JunoPL::Tests::FileHandlerTest fileHandler;
    fileHandler.setSourceCode("program test;\n"
                             "instr main() {\n"
                             "    if (true) {\n"
                             "        var a = 1;\n"
                             "    }\n"
                             "    elif (false) {\n"
                             "    }\n"
                             "    else {\n"
                             "    }\n"
                             "}\n");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileHandler);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &tokens = lexOutput.tokens;
    const auto &function =
        std::get<JunoPL::FunctionNode>(parseOutput.root->body.at(0));
    REQUIRE(function.body.body.size() == 1);

    const auto &conditional = std::get<JunoPL::ConditionalStatement>(
        function.body.body.at(0)->statement);

    checkLocation(conditional.loc, findToken(tokens, JunoPL::TokenType::K_IF));
    CHECK(conditional.loc.line == 2);

    checkLocation(conditional.ifStatement.loc,
                  findToken(tokens, JunoPL::TokenType::K_IF));
    // Each branch body is located at its own opening brace
    checkLocation(conditional.ifStatement.body.loc,
                  findToken(tokens, JunoPL::TokenType::OP_CBRAC_OP, 1));
    CHECK(conditional.ifStatement.body.loc.line == 2);

    REQUIRE(conditional.elifStatements.size() == 1);
    checkLocation(conditional.elifStatements.at(0).loc,
                  findToken(tokens, JunoPL::TokenType::K_ELIF));
    CHECK(conditional.elifStatements.at(0).loc.line == 5);
    checkLocation(conditional.elifStatements.at(0).body.loc,
                  findToken(tokens, JunoPL::TokenType::OP_CBRAC_OP, 2));

    checkLocation(conditional.elseStatement.loc,
                  findToken(tokens, JunoPL::TokenType::OP_CBRAC_OP, 3));
    CHECK(conditional.elseStatement.loc.line == 7);
}

TEST_CASE("Source Location - loops and break") {
    JunoPL::Tests::FileHandlerTest fileHandler;
    fileHandler.setSourceCode("program test;\n"
                             "instr main() {\n"
                             "    repeat 3 {\n"
                             "        break;\n"
                             "    }\n"
                             "    while (true) {\n"
                             "    }\n"
                             "    for i from 0 to 10 by 1 {\n"
                             "    }\n"
                             "}\n");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileHandler);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &tokens = lexOutput.tokens;
    const auto &function =
        std::get<JunoPL::FunctionNode>(parseOutput.root->body.at(0));
    REQUIRE(function.body.body.size() == 3);

    const auto &repeat =
        std::get<JunoPL::RepeatLoop>(function.body.body.at(0)->statement);
    checkLocation(repeat.loc, findToken(tokens, JunoPL::TokenType::K_REPEAT));
    CHECK(repeat.loc.line == 2);

    // Break lives inside the repeat body
    REQUIRE(repeat.body.body.size() == 1);
    const auto &breakStmt =
        std::get<JunoPL::BreakStatement>(repeat.body.body.at(0)->statement);
    checkLocation(breakStmt.loc, findToken(tokens, JunoPL::TokenType::K_BREAK));
    CHECK(breakStmt.loc.line == 3);

    const auto &whileLoop =
        std::get<JunoPL::WhileLoop>(function.body.body.at(1)->statement);
    checkLocation(whileLoop.loc, findToken(tokens, JunoPL::TokenType::K_WHILE));
    CHECK(whileLoop.loc.line == 5);

    const auto &forLoop =
        std::get<JunoPL::ForLoop>(function.body.body.at(2)->statement);
    checkLocation(forLoop.loc, findToken(tokens, JunoPL::TokenType::K_FOR));
    CHECK(forLoop.loc.line == 7);
}

TEST_CASE("Source Location - expressions") {
    JunoPL::Tests::FileHandlerTest fileHandler;
    fileHandler.setSourceCode(
        "program test;\n"
        "expr e() x * -y + z ? foo(42, items[0]) else 3;\n");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileHandler);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &tokens = lexOutput.tokens;
    const auto &customExpr =
        std::get<JunoPL::CustomExpression>(parseOutput.root->body.at(0));
    REQUIRE(std::holds_alternative<JunoPL::TernaryOp>(
        customExpr.expression->value));

    // Ternary reuses its condition's location (top-level addition)
    const auto &ternary =
        std::get<JunoPL::TernaryOp>(customExpr.expression->value);
    checkLocation(ternary.loc, findToken(tokens, JunoPL::TokenType::OP_PLUS));

    // Binary and unary operators are located at the operator token
    REQUIRE(std::holds_alternative<JunoPL::BinaryOp>(ternary.condition->value));
    const auto &addition =
        std::get<JunoPL::BinaryOp>(ternary.condition->value);
    checkLocation(addition.loc, findToken(tokens, JunoPL::TokenType::OP_PLUS));

    REQUIRE(std::holds_alternative<JunoPL::BinaryOp>(addition.lhs->value));
    const auto &multiplication =
        std::get<JunoPL::BinaryOp>(addition.lhs->value);
    checkLocation(multiplication.loc,
                  findToken(tokens, JunoPL::TokenType::OP_MULT));

    REQUIRE(std::holds_alternative<JunoPL::UnaryOp>(multiplication.rhs->value));
    const auto &negation = std::get<JunoPL::UnaryOp>(multiplication.rhs->value);
    checkLocation(negation.loc, findToken(tokens, JunoPL::TokenType::OP_MINUS));

    // Calls are located at the callee identifier, literals at their token
    REQUIRE(std::holds_alternative<JunoPL::CallExpression>(
        ternary.valueIfTrue->value));
    const auto &call =
        std::get<JunoPL::CallExpression>(ternary.valueIfTrue->value);
    checkLocation(call.loc, findIdent(tokens, "foo"));

    REQUIRE(call.params.size() == 2);
    const auto &literal = std::get<JunoPL::Value>(call.params.at(0)->value);
    checkLocation(literal.loc, findToken(tokens, JunoPL::TokenType::NUM, 0));

    // List indexing is located at the list identifier
    const auto &index = std::get<JunoPL::ListIndex>(call.params.at(1)->value);
    checkLocation(index.loc, findIdent(tokens, "items"));
}

TEST_CASE("Source Location - wrapper loc() accessors") {
    JunoPL::Tests::FileHandlerTest fileHandler;
    fileHandler.setSourceCode("program test;\n"
                             "instr main() {\n"
                             "    var x = 1;\n"
                             "}\n");

    JunoPL::Lexer lexer;
    auto lexOutput = lexer.tokenize(fileHandler);
    REQUIRE(lexOutput.errors.empty());

    JunoPL::Parser parser;
    auto parseOutput = parser.parse(lexOutput.tokens);

    CHECK(parseOutput.errors.empty());
    REQUIRE(parseOutput.root);

    const auto &function =
        std::get<JunoPL::FunctionNode>(parseOutput.root->body.at(0));
    REQUIRE(function.body.body.size() == 1);

    // Statements::loc() reports the wrapped statement's location
    const auto &stmt = *function.body.body.at(0);
    const auto &varDecl = std::get<JunoPL::VarDeclaration>(stmt.statement);
    checkLocation(stmt.loc(),
                  findToken(lexOutput.tokens, JunoPL::TokenType::K_VAR));
    CHECK(stmt.loc().line == 2);
    CHECK(stmt.loc().line == varDecl.loc.line);
    CHECK(stmt.loc().colStart == varDecl.loc.colStart);

    // Expression::loc() reports the wrapped expression's location
    REQUIRE(varDecl.value);
    const auto &value = std::get<JunoPL::Value>(varDecl.value->value);
    checkLocation(varDecl.value->loc(),
                  findToken(lexOutput.tokens, JunoPL::TokenType::NUM, 0));
    CHECK(varDecl.value->loc().line == value.loc.line);
    CHECK(varDecl.value->loc().colEnd == value.loc.colEnd);
}

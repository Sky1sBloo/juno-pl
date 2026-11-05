# C++ API Reference

All public APIs live in the `JunoPL` namespace. The project uses C++20
modules; each module is named `junopl.<subsystem>`.

## junopl.lexer — Tokenizer

### `class Lexer`

Tokenizes source text into a `TokenList`.

| Function | Description |
|----------|-------------|
| `Lexer()` | Constructs a lexer. |
| `Output tokenize(IFileHandler &fileHandler)` | Tokenizes the source from a file handler. Returns tokens plus any lexer errors. |
| `void reset()` | Resets the lexer state (line/column/lexeme). |

### `struct Lexer::Output`

| Member | Type | Description |
|--------|------|-------------|
| `tokens` | `TokenList` | The produced token stream. |
| `errors` | `std::vector<LexerError>` | Any lexing errors encountered. |

### `struct TokenList`

A vector of `Token` values. Defined in `junopl.lexer.tokens`.

## junopl.filehandler — File I/O abstraction

### `enum class FileHandlerError`

| Value | Meaning |
|-------|---------|
| `FileNotFound` | Input file does not exist. |
| `FileNotReadable` | Input file cannot be read. |
| `FileNotOpen` | Output file cannot be opened for writing. |
| `FileEOF` | End of file reached while reading. |
| `FileNotWritable` | Output file cannot be written. |

### `class IFileHandler` (interface)

| Function | Description |
|----------|-------------|
| `virtual std::expected<char, FileHandlerError> read() = 0` | Read one character. |
| `virtual std::expected<void, FileHandlerError> write(const std::string &data) = 0` | Write data. |
| `virtual ~IFileHandler() = default` | Virtual destructor. |

### `class FileHandler : public IFileHandler`

| Function | Description |
|----------|-------------|
| `static std::expected<FileHandler, FileHandlerError> open(const std::filesystem::path &path)` | Opens a file for reading. |
| `static std::expected<void, FileHandlerError> writeToFile(const std::filesystem::path &path, const std::string &data)` | Writes a string to a file. |

## junopl.parser — Parser and AST

### `class Parser`

Parses a token stream into a `RootNode` AST.

| Function | Description |
|----------|-------------|
| `Output parse(TokenList &tokens)` | Full parse: program → AST root. Returns root plus any parser errors. |

### `struct Parser::Output`

| Member | Type | Description |
|--------|------|-------------|
| `root` | `std::unique_ptr<RootNode>` | The parsed AST root (null on failure). |
| `errors` | `std::vector<ParserError>` | Any parse errors. |

### Individual parse methods (public for testing)

| Function | Description |
|----------|-------------|
| `std::optional<FunctionNode> parseFunction(TokenList &)` | Parses an `instr` definition. |
| `std::optional<CustomExpression> parseCustomExpression(TokenList &)` | Parses an `expr` definition. |
| `std::optional<EventNode> parseEvent(TokenList &)` | Parses an `on <event>` handler. |
| `std::optional<VarDeclaration> parseVarDeclaration(TokenList &)` | Parses a `var` declaration. |
| `std::optional<VarAssignment> parseVarAssignment(TokenList &)` | Parses an assignment statement. |
| `std::optional<DoInstruction> parseQualifiedInstruction(TokenList &)` | Parses a qualified `do <Q>.name(...)`. |
| `std::optional<ListDeclaration> parseListDeclaration(TokenList &)` | Parses a `list` declaration. |
| `std::optional<DoInstruction> parseDoInstruction(TokenList &)` | Parses an unqualified `do name(...)`. |
| `std::optional<BreakStatement> parseBreakStatement(TokenList &)` | Parses `break`. |
| `std::optional<ConditionalStatement> parseConditionalStatement(TokenList &)` | Parses `if`/`elif`/`else`. |
| `std::optional<RepeatLoop> parseRepeatLoop(TokenList &)` | Parses `repeat N { ... }`. |
| `std::optional<WhileLoop> parseWhileLoop(TokenList &)` | Parses `while (c) { ... }`. |
| `std::optional<ForLoop> parseForLoop(TokenList &)` | Parses `for (i = a; i < b; i += c) { ... }`. |
| `std::optional<EmitEvent> parseEmitEvent(TokenList &)` | Parses `emit name(...) to ...`. |
| `std::optional<std::vector<std::string>> parseParam(TokenList &)` | Parses a comma-separated parameter list. |
| `std::optional<std::vector<ExpressionHandle>> parseParamExpr(TokenList &)` | Parses comma-separated expression arguments. |
| `std::optional<Body> parseBody(TokenList &)` | Parses a `{ ... }` block. |
| `std::optional<ExpressionHandle> parseExpression(TokenList &)` | Parses a single expression. |

### AST node types

Defined in `junopl.parser.nodes`, `junopl.parser.nodes.statements`,
`junopl.parser.nodes.expressions`.

#### `RootNode`
| Member | Type | Description |
|--------|------|-------------|
| `programName` | `std::string` | Name from `program <name>;`. |
| `body` | `std::vector<RootNodes>` | Top-level declarations. |
| `loc` | `SourceLocation` | Source location. |

`RootNodes` is a variant of: `ImportNode`, `EventNode`, `FunctionNode`,
`VarDeclaration`, `CustomExpression`, `ListDeclaration`.

#### `EventNode`
| Member | Type | Description |
|--------|------|-------------|
| `identifier` | `std::string` | Event name (e.g. `"start"`, `"message"`). |
| `params` | `std::vector<std::string>` | Event parameter names. |
| `body` | `Body` | Handler body. |
| `loc` | `SourceLocation` | Source location. |

#### `FunctionNode` (custom instruction)
| Member | Type | Description |
|--------|------|-------------|
| `identifier` | `std::string` | Instruction name. |
| `params` | `std::vector<std::string>` | Parameter names. |
| `body` | `Body` | Instruction body. |
| `loc` | `SourceLocation` | Source location. |

#### `CustomExpression`
| Member | Type | Description |
|--------|------|-------------|
| `identifier` | `std::string` | Expression name. |
| `params` | `std::vector<std::string>` | Parameter names. |
| `expression` | `ExpressionHandle` | Single-expression body. |
| `loc` | `SourceLocation` | Source location. |

#### `VarDeclaration`
| Member | Type | Description |
|--------|------|-------------|
| `identifier` | `std::string` | Variable name. |
| `value` | `ExpressionHandle` | Optional initializer. |
| `loc` | `SourceLocation` | Source location. |

#### `ListDeclaration`
| Member | Type | Description |
|--------|------|-------------|
| `identifier` | `std::string` | List name. |
| `values` | `std::vector<ExpressionHandle>` | Initial list elements. |
| `loc` | `SourceLocation` | Source location. |

#### `Body`
| Member | Type | Description |
|--------|------|-------------|
| `body` | `std::vector<StatementsHandle>` | Statements in the block. |
| `loc` | `SourceLocation` | Source location. |

#### `Statements` (variant)
One of: `Body`, `VarDeclaration`, `VarAssignment`, `ListDeclaration`,
`DoInstruction`, `BreakStatement`, `ConditionalStatement`,
`EmitEvent`, `RepeatLoop`, `WhileLoop`, `ForLoop`.

#### `VarAssignment`
| Member | Type | Description |
|--------|------|-------------|
| `identifier` | `std::string` | Target variable. |
| `indexExpr` | `ExpressionHandle` | Optional list index (for `nums[0] = ...`). |
| `value` | `ExpressionHandle` | Assigned value. |
| `op` | `enum { ASSIGN, PLUS_ASSIGN, MINUS_ASSIGN, MULT_ASSIGN, DIVIDE_ASSIGN }` | Assignment operator. |
| `loc` | `SourceLocation` | Source location. |

#### `DoInstruction`
| Member | Type | Description |
|--------|------|-------------|
| `qualifier` | `std::string` | STL qualifier (empty for custom instr). |
| `identifier` | `std::string` | Instruction name. |
| `params` | `std::vector<ExpressionHandle>` | Arguments. |
| `loc` | `SourceLocation` | Source location. |

#### `ConditionalStatement`
| Member | Type | Description |
|--------|------|-------------|
| `ifStatement` | `IfStatement` | The `if` branch. |
| `elifStatements` | `std::vector<IfStatement>` | `elif` branches. |
| `elseStatement` | `Body` | `else` branch (may be empty). |
| `loc` | `SourceLocation` | Source location. |

#### `EmitEvent`
| Member | Type | Description |
|--------|------|-------------|
| `identifier` | `std::string` | Event name to broadcast. |
| `params` | `std::vector<ExpressionHandle>` | Event parameters. |
| `option` | `enum { TO_SCRIPT, TO_CRAFT, TO_NEARBY_CRAFTS }` | Broadcast target. |
| `loc` | `SourceLocation` | Source location. |

#### `RepeatLoop` / `WhileLoop` / `ForLoop`
| Member | Type | Description |
|--------|------|-------------|
| `repeatNumber` / `condition` / `start`,`end`,`by` | `ExpressionHandle` | Loop control expressions. |
| `iterator` | `std::string` | For-loop variable name (ForLoop only). |
| `body` | `Body` | Loop body. |
| `loc` | `SourceLocation` | Source location. |

### Expressions (variant in `Expression::value`)

#### `Value`
| Member | Type | Description |
|--------|------|-------------|
| `value` | `std::string` | Literal text or identifier name. |
| `kind` | `enum class Kind { Literal, Identifier }` | Distinguishes literals from identifiers. |
| `loc` | `SourceLocation` | Source location. |

#### `CallExpression`
| Member | Type | Description |
|--------|------|-------------|
| `qualifier` | `std::string` | STL qualifier (empty for custom expr call). |
| `identifier` | `std::string` | Function/expression name. |
| `params` | `std::vector<ExpressionHandle>` | Arguments. |
| `loc` | `SourceLocation` | Source location. |

#### `BinaryOp`
| Member | Type | Description |
|--------|------|-------------|
| `operation` | `enum class Operation` | `ADD, SUBTRUCT, MULTIPLY, DIVIDE, MOD, AND, OR, EQUAL, NOT_EQUAL, GREATER, LESS, GREATER_EQ, LESS_EQ`. |
| `lhs` / `rhs` | `ExpressionHandle` | Operands. |
| `loc` | `SourceLocation` | Source location. |

#### `UnaryOp`
| Member | Type | Description |
|--------|------|-------------|
| `operation` | `enum class Operation { NOT, NEGATE }` | Operator. |
| `value` | `ExpressionHandle` | Operand. |
| `loc` | `SourceLocation` | Source location. |

#### `ListIndex`
| Member | Type | Description |
|--------|------|-------------|
| `identifier` | `std::string` | List name. |
| `index` | `ExpressionHandle` | Index expression (0-based in Juno). |
| `loc` | `SourceLocation` | Source location. |

#### `TernaryOp`
| Member | Type | Description |
|--------|------|-------------|
| `condition` | `ExpressionHandle` | Condition. |
| `valueIfTrue` / `valueIfFalse` | `ExpressionHandle` | Branches. |
| `loc` | `SourceLocation` | Source location. |

### `class SourceLocation`

| Member | Type | Description |
|--------|------|-------------|
| `line` | `int` | Source line (-1 if unknown). |
| `colStart` | `int` | Start column. |
| `colEnd` | `int` | End column. |
| `bool isValid()` | | True when `line >= 0`. |

Defined in `junopl.common.location`.

## junopl.semantic — Semantic analysis

### `class SemanticAnalyzer`

Performs name resolution and arity checking over the AST.

| Function | Description |
|----------|-------------|
| `std::vector<Diagnostic> analyze(const RootNode &root)` | Runs both passes over the program. Pass 1 hoists all top-level symbol declarations; Pass 2 visits bodies/expressions. Returns all diagnostics found. |

Forward references to `expr` and `instr` are supported because of the
two-pass hoisting.

### `struct Diagnostic`
| Member | Type | Description |
|--------|------|-------------|
| `message` | `std::string` | Human-readable diagnostic message. |
| `loc` | `SourceLocation` | Source location of the issue. |

### `struct Symbol`
| Member | Type | Description |
|--------|------|-------------|
| `name` | `std::string` | Symbol name. |
| `kind` | `enum class Kind { VALUE, LIST }` | Variable or list. |
| `arity` | `std::size_t` | Parameter count (for callables). |
| `declaredAt` | `SourceLocation` | Where declared. |

Defined in `junopl.semantic.symbols`.

## junopl.compiler — Code generation

### `class Compiler`

Lowers the AST to Vizzy XML.

| Function | Description |
|----------|-------------|
| `std::string compileAST(const RootNode &root)` | Traverses the AST and returns compiled Vizzy XML as a string. |
| `std::expected<void, FileHandlerError> writeToFile(const std::filesystem::path &path, const std::string &xml)` | Writes compiled XML to disk. |
| `std::map<std::string, std::string> getVariables(const RootNode &root)` | Collects all variable declarations into a key → emitted-name map. Keyed by scope path + name; scoped names are hashed (e.g. `i_9db4c1a2`). |

### `struct Compiler::getVariables` return value

The map maps scope-qualified keys to emitted Vizzy variable names:
- Top-level variable `x` → key `"x"`, value `"x"`
- Scoped variable `x` in handler `main` statement 0 → key `"main.0.x"`,
  value `"x_<hash>"`
- The hash keeps same-named variables from different scopes apart.
- A generated name already taken gains a counter suffix.

## junopl.stl — Standard library registry

### `class JunoSTL`

Registry of all standard-library functions with qualifiers, operations,
and arity ranges.

#### Nested types

| Type | Description |
|------|-------------|
| `enum class Qualifiers` | `COMMON, MATH, STRINGS, INPUT, LISTS, VECTOR, GAME, CRAFT, PART, CAMERA, NAV, ALTITUDE, VELOCITY, ORBIT, PLANET, ATMOSPHERE, FUEL, TIME, PERFORMANCE` |
| `enum class Operations` | `INSTRUCTION` (used with `do`), `EXPRESSION` (used in expressions) |
| `static constexpr std::size_t kUnbounded` | Variadic upper bound (e.g. for `String.join`). |
| `struct ArityRange { std::size_t min; std::size_t max; }` | Inclusive parameter-count range. |
| `struct Function` | Registered function: qualifier, operation, identifier, arities. |

#### `struct Function` methods

| Method | Description |
|--------|-------------|
| `bool accepts(std::size_t argCount) const` | True when `argCount` falls in any registered arity range. |
| `std::string describeArities() const` | Human-readable arity summary (e.g. `"1 or 2 arguments"`). |

#### Static methods

| Method | Description |
|--------|-------------|
| `static Qualifiers qualifierFromString(const std::string &)` | Converts `"Math"`, `"String"`, etc. to enum. Unknown → `COMMON`. |
| `static std::string qualifierToString(Qualifiers)` | Converts enum back to string form. |
| `static std::optional<Function> find(Qualifiers, const std::string &ident)` | Finds first registered signature for a (qualifier, name) pair. |
| `static std::optional<Function> find(const std::string &qualifier, const std::string &ident)` | String-qualifier variant. |
| `static std::optional<Function> find(Qualifiers, const std::string &ident, Operations op)` | Finds signature matching qualifier, name, **and** operation type. |
| `static std::optional<Function> find(const std::string &qualifier, const std::string &ident, Operations op)` | String-qualifier, operation-aware variant. |

The operation-aware overload is needed because some names exist in both
contexts (e.g. `Input.throttle(x)` as an instruction and
`Input.throttle()` as an expression).

## junopl.common.location — Source location

### `struct SourceLocation`

| Member | Type | Description |
|--------|------|-------------|
| `int line = -1` | Source line. |
| `int colStart = -1` | Start column. |
| `int colEnd = -1` | End column. |
| `bool isValid() const` | True when `line >= 0`. |

A default-constructed location means "unknown".

## Example
```cpp
import junopl.lexer;
import junopl.parser;
import junopl.semantic;
import junopl.compiler;
import junopl.filehandler;

// 1. Read input
auto file = JunoPL::FileHandler::open("input.juno");

// 2. Lex
JunoPL::Lexer lexer;
auto lexOut = lexer.tokenize(*file);

// 3. Parse
JunoPL::Parser parser;
auto parseOut = parser.parse(lexOut.tokens);

// 4. Semantic analysis
JunoPL::SemanticAnalyzer analyzer;
auto diags = analyzer.analyze(*parseOut.root);

// 5. Compile to XML
JunoPL::Compiler compiler;
compiler.getVariables(*parseOut.root);
std::string xml = compiler.compileAST(*parseOut.root);

// 6. Write output
compiler.writeToFile("output.xml", xml);
```

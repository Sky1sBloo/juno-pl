# JunoPL 

JunoPL is a programming language that compiles to Vizzy XML, the visual
programming language used in *Juno New Origins*. The Juno compiler
(`juno-pl`) reads `.juno` source files and emits `.xml` files that can be
loaded into the game's Vizzy.

## Documentation
1. [Api Reference](docs/api-reference.md)
2. [Language Reference](docs/language-reference.md)
3. [Standard Library](docs/stdlib-reference.md)

## Repository layout
```
juno-lang/
├── docs/                 # This documentation
├── src/
│   ├── common/           # SourceLocation type
│   ├── compiler/         # AST → Vizzy XML code generation
│   ├── file-handler/     # File reading/writing abstraction
│   ├── juno-stl/         # Standard library function registry
│   ├── lexer/            # Tokenizer
│   ├── parser/           # Parser and AST node definitions
│   ├── semantic-analyzer/# Name resolution and type/arity checking
│   └── main.cpp          # CLI entry point
├── tests/                # Unit tests (doctest)
├── build.sh              # Build script (cmake + ninja)
└── CMakeLists.txt        # CMake build configuration
```

## Building

```sh
./build.sh
```

This runs `cmake -B build -DCMAKE_CXX_COMPILER=clang++ -G Ninja` followed by
`ninja -C build`. The `juno-pl` binary is produced in `build/`.

## CLI usage

```
juno-pl <input.juno> [output.xml]
```

- `input.juno` — required source file
- `output.xml` — optional output path; defaults to `input.xml` next to the
  source

The compiler pipeline is: **lex → parse → semantic analyze → compile to XML
→ write file**. Errors at any stage are printed to stderr and the process
exits with code 1.

## Example

`hello.juno`:
```
program hello;
var message = "Hello, Kerbin!";

on start() {
    Game.display(message);
}
```

Compile:
```sh
./build/juno-pl hello.juno hello.xml
```

The produced `hello.xml` contains Vizzy XML ready to import into SimpleRockets 2.

## Modules

| Module | Purpose |
|--------|---------|
| `junopl.lexer` | Tokenizes source text into a token stream |
| `junopl.parser` | Parses tokens into an AST (`RootNode`) |
| `junopl.semantic` | Resolves names, checks arities, reports diagnostics |
| `junopl.compiler` | Lowers the AST to Vizzy XML |
| `junopl.filehandler` | File read/write abstraction (`IFileHandler`, `FileHandler`) |
| `junopl.stl` | Standard library function registry (`JunoSTL`) |

See `docs/api-reference.md` for per-module function documentation and
`docs/stdlib-reference.md` for the standard library function catalog.

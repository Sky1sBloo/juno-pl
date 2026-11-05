# Juno Language Reference

## Program structure

A Juno source file begins with a `program` declaration, followed by
top-level declarations:

```
program <name>;

<top-level declaration>
...
```

Top-level declarations can be:
- Variable declarations: `var x = 5;`
- List declarations: `list engines = [1, 2, 3];`
- Event handlers: `on start() { ... }`
- Custom expressions: `expr add(a, b) a + b;`
- Custom instructions: `instr logMsg(msg) { ... }`

## Variables and lists

Vizzy has no local custom variables internally but JunoPL supports it. Local variables in vizzy are used from function parameters or message data blocks (the yellow ones). Variables
declared inside expression bodies are given hashed names (e.g. `x_9db4c1a2`) to avoid
name collisions across scopes. 

### Variable declaration
```
var <name> = <expression>;
var <name>;              // defaults to 0
```

### List declaration
```
list <name> = [<expr>, <expr>, ...];
list <name>;             // empty list
```

Top-level variable initializers (e.g. `var x = 5;`) are emitted as
assignments at the top of the `start` handler (or a synthesized
`FlightStart` handler when no `start` exists), before any user statements.

## Events
Event handlers respond to Vizzy events:

```
on start() {
    // runs when the craft launches
}

on message(a, b) {
    // custom event; params a and b arrive via Vizzy's `data` list,
    // indexed data[1], data[2]
    // called with emit message(<expr>, <expr>);
}
```

Custom event parameters are referenced through Vizzy's built-in `data`
list using 1-based positions: the first parameter is `data[1]`, the second
is `data[2]`, etc.

### Events vs Instructions
The difference of events compared to instructions is that they can run concurrently from the start event as well as be called from other script.
It is emitted internally as `broadcast message` and `on message receive` in game.

## Expressions

### Literals
```
42              // number
"hello"         // string
true            // boolean
false           // boolean
```

### Identifiers
```
x               // variable reference (resolved to hashed global name)
```

### Binary operators
```
a + b           // addition
a - b           // subtraction
a * b           // multiplication
a / b           // division
a % b           // modulo
a == b          // equality
a != b          // inequality
a > b           // greater than
a < b           // less than
a >= b          // greater than or equal
a <= b          // less than or equal
a and b         // logical and
a or b          // logical or
```

### Unary operators
```
-a              // negation
not a           // logical not
```

### List indexing (0-based in Juno, 1-based in Vizzy)
```
nums[0]         // first element; emitted as data[1] in Vizzy
```

### Ternary conditional
```
condition ? trueValue : falseValue
```

### Function calls
```
Math.abs(-5)           // standard library expression
Input.throttle(0.5)    // standard library expression
add(a, b)              // custom expression call
```

## Statements

### Simple assignment
```
x = 5;
```

### Compound assignment
```
x += 1;        // ChangeVariable
x -= 2;        // ChangeVariable (amount negated)
x *= 3;        // SetVariable with BinaryOp
x /= 4;        // SetVariable with BinaryOp
```

### List element assignment
```
nums[0] = 5;           // SetList op="set"
nums[1] += 2;          // read-modify-write via SetList
```

### Variable declaration inside a handler
```
var y = 10;
```

### If / elif / else
```
if (x > 0) {
    ...
} elif (x < 0) {
    ...
} else {
    ...
}
```

### Loops
```
repeat 5 {
    ...
}

while (x > 0) {
    ...
}

/**
This internally converts to a while loop with a custom local variable to allow multiple types of iterator i.
*/
for i from 0 to 10 by 1 {
    ...
}
```

`break` exits the innermost loop.

### Do instruction
```
do Game.delay(5);
do Craft.activateStage();
do Input.throttle(0.5);
do logMsg("hello");     // custom instruction call
```

### Emit event
```
emit message() to script;
emit message(a, b) to craft;
emit message(a, b) to nearby;
```

`to script` — broadcast to the part running the program
`to craft` — broadcast to the target craft
`to nearby` — broadcast to all nearby crafts

## Custom expressions

```
expr <name>(<param1>, <param2>, ...) <expression>;
```

Single-expression body (no braces, no `return`):

```
expr add(a, b) a + b;
expr double(x) x * 2;
```

Called as `add(a, b)`. Custom expressions are emitted as Vizzy
`CallCustomExpression` blocks.

## Custom instructions

```
instr <name>(<param1>, ...) {
    <statement>
    ...
}
```

Example:
```
instr logMsg(msg) {
    Game.log(msg, true);
}
```

Called with `do logMsg("hello");`.

## Forward references

`expr` and `instr` definitions are **hoisted** — they can be referenced
from handlers that appear earlier in the source:

```
on start() {
    do greet();   // greet is defined below
}

instr greet() {
    Game.display("hi");
}
```

## Top-level initializers

Variables and lists declared at the top level with initial values are
assigned at the top of the `start` handler (or a synthesized `FlightStart`
handler when no `start` exists), before any user statements:

```
var thrust = 0;
list engines = [1, 2, 3];

on start() {
    thrust = 100;
}
```

Emits `thrust = 0` and `engines = [1,2,3]` at the top of `start`, then
`thrust = 100`.

## Standard library

Qualified calls use the form `<Qualifier>.<name>(<args>)`. Qualifiers:
`Math`, `String`/`Strings`, `Input`, `Lists`, `Vector`, `Game`, `Craft`,
`Part`, `Camera`, `Nav`, `Altitude`, `Velocity`, `Orbit`, `Planet`,
`Atmosphere`, `Fuel`, `Time`, `Performance`, `Terrain`, `ListOp`, `Raycast`,
`Target`, `Misc`.

See `docs/stdlib-reference.md` for the complete function catalog with
signatures and descriptions.

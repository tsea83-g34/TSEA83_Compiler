# C-- Compiler

This is a compiler written for custom hardware designed in the course TSEA83 at Linköpings Universitet.

## Development Notes

### Lexer

- [x] Implement a lexer that can identify tokens of the following language features
    - Variable declarations
    - Function declarations
    - If statements
    - Block statements
    - Variable assignment
    - Arithmetic operators `+, -`
    - Relational operators `==, !=`

### Parsing

- [x] Parsing for the features listed above
- [x] Data structures that represent different language constructs such as expressions and statements
- [x] Build abstract tree with these data structures from the stream of tokens provided by the lexer

### Code Generation

- [ ] Symbol table tree for variables and declarations
- [ ] Register allocation table and register usage
- [ ] Translation from abstract syntax tree to machine code

### Other Language Features

- [ ] Structs, struct operator `.`
- [ ] Pointers, pointer operators `*, &, ->, []`
- [ ] While-loops
- [ ] For-loops
- [ ] Arithmetic operators `*, %, a++, ++a, a--, --a, +=, -=, *=`
- [ ] Logical operators `&&, ||, ^, !`
- [ ] Bitwise operators `~, &, |, ^, <<, >>`
- [ ] Compound assignment operators 

## C-- Specification

### Types


#### Integer types
    - char (signed 8-bit)
    - int  (signed 16-bit)
    - long (signed 32-bit)

#### String types
    - const char* (string literal type)
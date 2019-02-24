# C-- Compiler

This is a compiler written for custom hardware designed in the course TSEA83 at Linköpings Universitet.

## Development Notes

### Lexer

- [x] Implement a lexer that can identify tokens of the following language features
    - If statements
    - Variable declarations
    - Function declarations
    - Block statements
    - Variable assignment
    - Arithmetic operators `+, -`
    - Relational operators `==, !=`

### Parsing

- [ ] Parsing for the features listed above
- [ ] Data structures that represent different language contstructs such as expressions and statements
- [ ] Build abstract tree with these data structures from the stream of tokens provided by the lexer
- [ ] Symbol table tree for variables and declarations

### Code Generation

- [ ] Translation from abstract syntax tree to machine code
- [ ] Register allocation table and register usage

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

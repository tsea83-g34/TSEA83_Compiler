# C-- Compiler

This is a compiler written for custom hardware designed in the course TSEA83 at Linköpings Universitet.

## Development Notes

### Lexer

[] Implement a lexer that can identify tokens of the following language features
    - If statements
    - Variable declarations
    - Function declarations
    - Block statements
    - Arithmetic operators + and -
    - Comparison operators == and !=

### Parsing

[] Implement parsing for the features listed above
[] Implement data structures that represent different language contstructs such as expressions and statements
[] Build abstract tree with these data structures from the stream of tokens provided by the lexer
[] Implement symbol table tree for variables and declarations

### Code Generation

[] Implement translation from abstract syntax tree to machine code
[] Implement a register allocation table and register usage

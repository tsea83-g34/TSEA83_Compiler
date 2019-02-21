# C-- Compiler

This is a compiler written for custom hardware designed in the course TSEA83 at Linköpings Universitet.

## Development Notes

### Lexer

    1. Implement a lexer that can identify tokens of the following language features
        - If statements
        - Variable declarations
        - Function declarations
        - Block statements
        - Arithmetic operators + and -
        - Comparison operators == and !=

### Parsing

    2. Implement parsing for the features listed above
    3. Implement data structures that represent different language contstructs such as expressions and statements
    4. Build abstract tree with these data structures from the stream of tokens provided by the lexer
    5. Implement symbol table tree for variables and declarations
    
### Code Generation

    7. Implement translation from abstract syntax tree to machine code
    8. Implement a register allocation table and register usage

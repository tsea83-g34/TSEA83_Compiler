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

- [x] Symbol table tree for variables and declarations
- [x] Register allocation table and register usage
- [x] Translation from abstract syntax tree to machine code

### Other Language Features

- [x] While loops
- [x] Else statements
- [x] Logical operators `&, |, !`
- [x] Extended relational operators `<, >, <=, >=`
- [x] Multiplication operator `*`
- [ ] Inline Assembly
- [ ] Pointers, pointer operators `*, &, []`
- [ ] Shift operators `<< >>`
- [ ] Headers and file includes
- [ ] Structs, struct operator `.`
- [ ] For-loops

## C-- Specification

### Grammar

    e denotes empty string

    program     ->  decls

    decls       ->  decl decls
                |   e

    decl        ->  func_decl
                |   var_decl
    
    var_decl    ->  type id ";"
                |   type id "=" expr ";"

    func_decl   ->  type id "(" param_decls ")" ";"
                |   type id "(" param_decls ")" block_stmt

    param_decls ->  param_decl param_decls
                |   e

    param_decl  ->  type id

    stmt        ->  block_stmt
                |   "if" "(" expr ")" stmt
                |   "if" "(" expr ")" stmt "else" stmt
                |   var_decl 
                |   id "=" expr ";"
                |   return expr ";"
                |   expr ";"
    
    stmts       ->  stmt stmts
                |   e

    block_stmt  ->  "{" stmts "}"

    expr        ->  term binop expr
                |   "-" term
                |   "!" term
                |   term

    binop       ->  "+"
                |   "-"
                |   "*"
                |   "=="
                |   "!="
                |   ">="
                |   "<="
                |   "&"
                |   "|"

    term        ->  id
                |   literal
                |   id "(" params ")"  // Function call
    
    params      ->  expr params
                |   e



### Types


#### Integer types
    - char (signed 8-bit)
    - int  (signed 16-bit)
    - long (signed 32-bit)

#### String types
    - const char* (string literal type)
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

### Grammar

    e denotes empty string

    program     ->  decls                       PROGRAM::1

    decls       ->  decl decls                  DECLS::1
                |   e                           DECLS::2

    decl        ->  func_decl
                |   var_decl
    
    var_decl    ->  type id ;
                |   type id "=" expr ;

    func_decl   ->  type id ( param_decls ) ;
                |   type id ( param_decls ) block_stmt

    param_decls ->  param_decl param_decls
                |   e

    param_decl  ->  type id

    stmt        ->  block_stmt
                |   if ( expr ) stmt // This could lead to great errors if expr is matched but not stmt?
                |   var_decl ;
                |   id "=" expr ;   // assignment
                |   return expr ;
                |   expr ;
    
    stmts       ->  stmt stmts
                |   e

    block_stmt  ->  { stmts }

    expr        ->  term binop expr
                |   "-" term
                |   term

    binop       ->  "+"
                |   "-"
                |   "=="
                |   "!="

    term        ->  id
                |   literal
                |   id ( params )  // Function call
    
    params      ->  expr params
                |   e



### Types


#### Integer types
    - char (signed 8-bit)
    - int  (signed 16-bit)
    - long (signed 32-bit)

#### String types
    - const char* (string literal type)
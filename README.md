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
- [x] Inline Assembly
- [x] Pointers, pointer operators `*, &, []`
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
    
    var_decl    ->  type id ;
                |   type * id ;
                |   type id = expr ;
                |   type * id = expr ;

    array_decl  ->  type id [ expr ] ;
                |   type id [ ] = { init_list } ;
                |   char id [ ] = str_lit ;

    init_list   ->  expr init_list
                |   e
            
    func_decl   ->  type id ( param_decls ) ;
                |   type id ( param_decls ) block_stmt

    param_decls ->  param_decl param_decls
                |   e

    param_decl  ->  type id

    stmt        ->  block_stmt
                |   if ( expr ) stmt
                |   if ( expr ) stmt else stmt
                |   while ( expr ) stmt
                |   asm ( str_lit asm_params ) ;
                |   continue literal ;
                |   break literal ; 
                |   var_decl 
                |   array_decl
                |   id = expr ;
                |   * id = expr ;
                |   id [ expr ] = expr ;
                |   return expr ;
                |   expr ;
    
    stmts       ->  stmt stmts
                |   e

    block_stmt  ->  { stmts }

    expr        ->  term binop expr
                |   - term
                |   ! term
                |   term

    binop       ->  +
                |   -
                |   *
                |   ==
                |   !=
                |   >=
                |   <=
                |   &
                |   |

    term        ->  id
                |   literal
                |   id ( params )
                |   ( expr )
                |   * id
                |   & id
                |   id [ expr ]
    
    params      ->  expr params
                |   e

    asm_params  ->  asm_param asm_params
                |   e
    
    asm_param   ->  id
                |   literal


### Inline assembly

Usage example:

    long inportb(int port) {
        long result = 0;
        asm ("in $, $" result port);
        return result;
    }

### Types

#### Integer types
    - char (signed 8-bit)
    - int  (signed 16-bit)
    - long (signed 32-bit)
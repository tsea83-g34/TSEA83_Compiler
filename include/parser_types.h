#ifndef COM_PARSER_TYPES_H
#define COM_PARSER_TYPES_H

#include <string>

#include "interfaces.h"

/* First tier C-- grammar

    e denotes empty string

    program     ->  decls                       PROGRAM::1

    decls       ->  decl decls                  DECLS::1
                |   e                           DECLS::2

    decl        ->  func_decl
                |   var_decl
    
    var_decl    ->  type id ;
                |   type "*" id ;
                |   type id "=" expr ;
                |   type "*" id "=" expr ;
    
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
                |   type * id

    stmt        ->  block_stmt
                |   if ( expr ) stmt
                |   if ( expr ) stmt else stmt
                |   while ( expr ) stmt
                |   asm ( str_lit params )
                |   var_decl
                |   array_decl
                |   id "=" expr ;   // assignment
                |   "*" id "=" expr ; // deref assignment
                |   id [ expr ] "=" expr ; // deref assignment
                |   return expr ;
                |   expr ;
    
    stmts       ->  stmt stmts
                |   e

    block_stmt  ->  { stmts }

    expr        ->  term binop expr
                |   "-" term
                |   "!" term
                |   "*" term
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
                |   id ( params )  // Function call
                |   ( expr )
                |   & id
                |   id [ expr ]
    
    params      ->  expr params
                |   e

    asm_params  ->  asm_param asm_params
                |   e
    
    asm_param   ->  id
                |   literal

 */

/* Predefine syntax tree structs */
struct program_t;

struct decl_t;
struct decls_t;
struct var_decl_t;
struct func_decl_t;

struct array_decl_t;

struct simple_array_decl_t;
struct init_list_array_decl_t;
struct str_array_decl_t;

struct init_list_t;

struct param_decls_t;
struct param_decl_t;
struct params_t;

struct stmt_t;
struct stmts_t;
struct block_stmt_t;
struct if_stmt_t;
struct while_stmt_t;
struct asm_stmt_t;
struct assignment_stmt_t;
struct deref_assignment_stmt_t;
struct indexed_assignment_stmt_t;
struct return_stmt_t;
struct expr_stmt_t;

struct expr_t;
struct neg_expr_t;
struct not_expr_t;;
struct term_expr_t;

struct term_t;
struct lit_term_t;
struct id_term_t;
struct call_term_t;
struct expr_term_t;
struct deref_term_t;
struct addr_of_term_t;
struct indexed_term_t;

struct asm_params_t;
struct asm_param_t;

// New binary operation system

struct binop_expr_t;

struct add_binop_t;
struct sub_binop_t;
struct and_binop_t;
struct or_binop_t;
struct mult_binop_t;

struct eq_binop_t;
struct neq_binop_t;
struct less_biop_t;
struct greater_binop_t;

struct less_eq_binop_t;
struct greater_eq_binop_t;

/* ----------------------------- */
class parser_t;
class translator_t;
struct func_info_t;

struct program_t : undoable_t, printable_t, translateable_t {
    decls_t* decls;

    program_t() = default;
    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
};

/*      Declarations      */
struct decls_t : undoable_t, printable_t, translateable_t {
    decl_t*   first;
    decls_t*  rest;

    decls_t() = default;
    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
};

struct decl_t : virtual undoable_t, virtual printable_t, virtual translateable_t {
    
    decl_t() = default;
};

struct func_decl_t : decl_t {
    int type;
    std::string id;
    param_decls_t* param_list;
    block_stmt_t* stmt;

    func_decl_t() = default;
    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
};

/* ---------------------- */

/*       Parameters       */

struct param_decls_t : undoable_t, virtual printable_t {
    param_decl_t* first;
    param_decls_t* rest;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t, func_info_t* f, int param_index);
};

struct param_decl_t : undoable_t, virtual printable_t {
    int type;
    std::string id;
    bool is_pointer;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t, func_info_t* f, int param_index);
};

struct params_t : undoable_t, virtual printable_t {
    expr_t* first;
    params_t* rest;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t, func_info_t* func, int param_index);
};

struct init_list_t : undoable_t, printable_t {

    expr_t* first;
    init_list_t* rest;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
};

struct asm_params_t : undoable_t, virtual printable_t, translateable_t {

    asm_param_t* first;
    asm_params_t* rest;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
};

struct asm_param_t : virtual undoable_t, virtual printable_t, virtual translateable_t { };


/* ---------------------- */

/*       Statements       */

struct stmt_t : virtual undoable_t, virtual printable_t, virtual translateable_t { };

struct var_decl_t : decl_t, stmt_t {
    int type;
    std::string id;
    bool is_pointer;
    expr_t* value;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
};

struct array_decl_t : decl_t, stmt_t {
    int type;
    std::string identifier;

};

struct simple_array_decl_t : array_decl_t {

    expr_t* size;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
};

struct init_list_array_decl_t : array_decl_t {
    
    init_list_t* init_list;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
};

struct str_array_decl_t : array_decl_t {

    std::string string_literal;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
};

struct stmts_t : undoable_t, printable_t, translateable_t {
    stmt_t* first;
    stmts_t* rest;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
};

struct block_stmt_t : stmt_t {
    stmts_t* statements;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
};

struct if_stmt_t : stmt_t {
    expr_t* cond;
    stmt_t* actions;
    stmt_t* else_actions;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
};

struct while_stmt_t : stmt_t {
    expr_t* cond;
    stmt_t* actions;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
};

struct asm_stmt_t : stmt_t {
    
    std::string literal;
    asm_params_t* params;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
};

struct assignment_stmt_t : stmt_t {
    std::string identifier;
    expr_t* rvalue;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
};

struct deref_assignment_stmt_t : stmt_t {
    
    std::string identifier;
    expr_t* rvalue;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
};

struct indexed_assignment_stmt_t : stmt_t {
    
    std::string identifier;
    expr_t* index;
    expr_t* rvalue;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
};

struct return_stmt_t : stmt_t {
    expr_t* return_value;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
};

struct expr_stmt_t : stmt_t {
    expr_t* e;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
};

/* ---------------------- */

struct expr_t : virtual undoable_t, virtual printable_t, virtual translateable_t {
    virtual bool evaluate(int* result) = 0;
};

struct neg_expr_t : expr_t {
    term_t* value;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
    bool evaluate(int* result) override;
};

struct not_expr_t : expr_t {
    term_t* value;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
    bool evaluate(int* result) override;
};

struct term_expr_t : expr_t {
    term_t* t;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
    bool evaluate(int* result) override;
};

struct term_t : expr_t {
    bool is_literal;

    virtual void undo(parser_t* p) override;
    virtual bool evaluate(int* result) = 0;
};

struct id_term_t : term_t, asm_param_t {
    std::string identifier;

    id_term_t() { is_literal = false; }
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
    bool evaluate(int* result) override;
};

struct call_term_t : term_t {
    std::string function_identifier;
    params_t* params;

    call_term_t() { is_literal = false; }
    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
    bool evaluate(int* result) override;
};

struct expr_term_t : term_t {
    expr_t* expr;

    expr_term_t() { is_literal = false; }
    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
    bool evaluate(int* result) override;
};

struct addr_of_term_t : term_t {
    std::string identifier;

    void undo(parser_t* p) override;
    addr_of_term_t() { is_literal = false; }
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
    bool evaluate(int* result) override;
};

struct deref_term_t : term_t {
    std::string identifier;

    void undo(parser_t* p) override;
    deref_term_t() { is_literal = false; }
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
    bool evaluate(int* result) override;
};

struct indexed_term_t : term_t {
    
    std::string identifier;
    expr_t* index;

    void undo(parser_t* p) override;
    indexed_term_t() { is_literal = false; }
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
    bool evaluate(int* result) override;
};

struct lit_term_t : term_t, asm_param_t {
    int literal;

    lit_term_t() { is_literal = true; }
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
    bool evaluate(int* result) override;
};

// New binary operation system

struct binop_expr_t : expr_t {
    
    // if left assoc this is the right term, if right assoc this is the left term
    expr_t* term; 
    
    // The rest
    expr_t* rest; 
    
    // Associativity
    bool left_assoc;

    void undo(parser_t* p) override;
    binop_expr_t() : term(nullptr), rest(nullptr), left_assoc(false) { }

    // Note that this transfers tokens aswell
    virtual binop_expr_t* duplicate() = 0;
    
    static expr_t* rewrite(expr_t* e);
};

// Arithmetic addition
struct add_binop_t : binop_expr_t {
    add_binop_t() : binop_expr_t() {}

    std::string get_string(parser_t* p) override;
    int translate(translator_t* p) override;
    bool evaluate(int* result) override;
    add_binop_t* duplicate() override;
};

// Arithmetic subtraction
struct sub_binop_t : binop_expr_t {
    sub_binop_t() : binop_expr_t() {}

    std::string get_string(parser_t* p) override;
    int translate(translator_t* p) override;
    bool evaluate(int* result) override;
    sub_binop_t* duplicate() override;
};

// Arithmetic multiplication
struct mult_binop_t : binop_expr_t {
    mult_binop_t() : binop_expr_t() {}

    std::string get_string(parser_t* p) override;
    int translate(translator_t* p) override;
    bool evaluate(int* result) override;
    mult_binop_t* duplicate() override;
};

// Logical and
struct and_binop_t : binop_expr_t {
    and_binop_t() : binop_expr_t() {}

    std::string get_string(parser_t* p) override;
    int translate(translator_t* p) override;
    bool evaluate(int* result) override;
    and_binop_t* duplicate() override;
};

// Logical or
struct or_binop_t : binop_expr_t {
    or_binop_t() : binop_expr_t() {}

    std::string get_string(parser_t* p) override;
    int translate(translator_t* p) override;
    bool evaluate(int* result) override;
    or_binop_t* duplicate() override;
};

// Relational equal
struct eq_binop_t : binop_expr_t {
    eq_binop_t() : binop_expr_t() {}

    std::string get_string(parser_t* p) override;
    int translate(translator_t* p) override;
    bool evaluate(int* result) override;
    eq_binop_t* duplicate() override;
};

// Relational not-equal
struct neq_binop_t : binop_expr_t {
    neq_binop_t() : binop_expr_t() {}

    std::string get_string(parser_t* p) override;
    int translate(translator_t* p) override;
    bool evaluate(int* result) override;
    neq_binop_t* duplicate() override;
};

// Relational less
struct less_binop_t : binop_expr_t {
    less_binop_t() : binop_expr_t() {}

    std::string get_string(parser_t* p) override;
    int translate(translator_t* p) override;
    bool evaluate(int* result) override;
    less_binop_t* duplicate() override;
};

// Relational greater
struct greater_binop_t : binop_expr_t {
    greater_binop_t() : binop_expr_t() {}

    std::string get_string(parser_t* p) override;
    int translate(translator_t* p) override;
    bool evaluate(int* result) override;
    greater_binop_t* duplicate() override;
};

// Relational less or equal
struct less_eq_binop_t : binop_expr_t {
    less_eq_binop_t() : binop_expr_t() {}

    std::string get_string(parser_t* p) override;
    int translate(translator_t* p) override;
    bool evaluate(int* result) override;
    less_eq_binop_t* duplicate() override;
};

// Relational greater or equal
struct greater_eq_binop_t : binop_expr_t {
    greater_eq_binop_t() : binop_expr_t() {}

    std::string get_string(parser_t* p) override;
    int translate(translator_t* p) override;
    bool evaluate(int* result) override;
    greater_eq_binop_t* duplicate() override;
};

#endif
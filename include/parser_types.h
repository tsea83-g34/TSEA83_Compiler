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

    expr        ->  term arithop expr
                |   "-" term
                |   term relop expr
                |   term

    arithop     ->  "+"
                |   "-"

    relop       ->  "=="
                |   "!="

    term        ->  id
                |   literal
                |   id ( params )  // Function call
    
    params      ->  expr params
                |   e

 */

/* Predefine syntax tree structs */
struct program_t;

struct decl_t;
struct decls_t;
struct var_decl_t;
struct func_decl_t;

struct param_decls_t;
struct param_decl_t;
struct params_t;

struct stmt_t;
struct stmts_t;
struct block_stmt_t;
struct if_stmt_t;
struct assignment_stmt_t;
struct return_stmt_t;
struct expr_stmt_t;

struct expr_t;
struct arith_expr_t;
struct rel_expr_t;
struct neg_expr_t;
struct term_expr_t;

struct term_t;
struct lit_term_t;
struct id_term_t;
struct call_term_t;

struct arithop_t;
struct relop_t;
/* ----------------------------- */
class parser_t;

struct program_t : undoable_t, printable_t {
    decls_t* decls;

    program_t() = default;
    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
};

/*      Declarations      */
struct decls_t : undoable_t, printable_t {
    decl_t*   first;
    decls_t*  rest;

    decls_t() = default;
    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
};

struct decl_t : virtual undoable_t, virtual printable_t {
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
};

/* ---------------------- */

/*       Parameters       */

struct param_decls_t : undoable_t, virtual printable_t {
    param_decl_t* first;
    param_decls_t* rest;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
};

struct param_decl_t : undoable_t, virtual printable_t {
    int type;
    std::string id;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
};

struct params_t : undoable_t, virtual printable_t {
    expr_t* first;
    params_t* rest;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
};

/* ---------------------- */

/*       Statements       */

struct stmt_t : virtual undoable_t, virtual printable_t { };

struct var_decl_t : decl_t, stmt_t {
    int type;
    std::string id;
    expr_t* value;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
};

struct stmts_t : undoable_t, printable_t {
    stmt_t* first;
    stmts_t* rest;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
};

struct block_stmt_t : stmt_t {
    stmts_t* statements;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
};

struct if_stmt_t : stmt_t {
    expr_t* cond;
    stmt_t* actions;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
};

struct assignment_stmt_t : stmt_t {
    std::string identifier;
    expr_t* rvalue;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
};

struct return_stmt_t : stmt_t {
    expr_t* return_value;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
};

struct expr_stmt_t : stmt_t {
    expr_t* e;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
};

/* ---------------------- */

struct expr_t : undoable_t, printable_t { };

struct arith_expr_t : expr_t {
    term_t* left;
    arithop_t* op;
    expr_t* right;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
};

struct rel_expr_t : expr_t {
    term_t* left;
    relop_t* op;
    expr_t* right;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
};

struct neg_expr_t : expr_t {
    term_t* value;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
};

struct term_expr_t : expr_t {
    term_t* t;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
};

struct term_t : undoable_t, printable_t {
    bool is_literal;

    virtual void undo(parser_t* p) override;
};

struct id_term_t : term_t {
    std::string identifier;

    id_term_t() { is_literal = false; }
    std::string get_string(parser_t* p) override;
};

struct call_term_t : term_t {
    std::string function_identifier;
    params_t* params;

    call_term_t() { is_literal = false; }
    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
};

struct lit_term_t : term_t {
    int literal;

    lit_term_t() { is_literal = true; }
    std::string get_string(parser_t* p) override;
};

struct arithop_t : undoable_t, printable_t {
    void undo(parser_t* p) override;
};

struct arithop_plus_t : arithop_t {
    std::string get_string(parser_t* p) override;
};

struct arithop_minus_t : arithop_t {
    std::string get_string(parser_t* p) override;
};

struct relop_t :  undoable_t, printable_t {
    void undo(parser_t* p) override;
};

struct relop_equals_t : relop_t {
    std::string get_string(parser_t* p) override;
};

struct relop_not_equals_t : relop_t {
    std::string get_string(parser_t* p) override;
};

#endif
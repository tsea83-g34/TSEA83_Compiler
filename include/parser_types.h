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

    func_decl   ->  type id ( ) ;
                |   type id ( ) block_stmt

    stmt        ->  block_stmt
                |   if ( expr ) stmt // This could lead to great errors if expr is matched but not stmt?
                |   var_decl ;
                |   id "=" expr ;   // assignment
    
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
 */

/* Predefine syntax tree structs */
struct program_t;

struct decl_t;
struct decls_t;
struct var_decl_t;
struct func_decl_t;

struct stmt_t;
struct stmts_t;
struct block_stmt_t;

struct if_stmt_t;
struct assignment_stmt_t;

struct expr_t;
struct arith_expr_t;
struct term_t;

struct arithop_t;
struct relop_t;
/* ----------------------------- */
class parser_t;

struct program_t : undoable_t {
    decls_t* decls;

    program_t() = default;
    void undo(parser_t* p) override;
};

/*      Declarations      */
struct decls_t : undoable_t {
    decl_t*   first;
    decls_t*  rest;

    decls_t() = default;
    void undo(parser_t* p) override;
};

struct decl_t : virtual undoable_t {
    decl_t  () = default;
};

struct func_decl_t : decl_t {
    int type;
    std::string id;
    block_stmt_t* stmt;

    func_decl_t() = default;
    void undo(parser_t* p) override;
};

/* ---------------------- */

/*       Statements       */

struct stmt_t : virtual undoable_t { };

struct var_decl_t : decl_t, stmt_t {
    int type;
    std::string id;
    expr_t* value;

    void undo(parser_t* p) override;
};

struct stmts_t : undoable_t {
    stmt_t* first;
    stmts_t* rest;

    void undo(parser_t* p) override;
};

struct block_stmt_t : stmt_t {
    stmts_t* statements;

    void undo(parser_t* p) override;
};

struct if_stmt_t : stmt_t {
    expr_t* cond;
    stmt_t* actions;

    void undo(parser_t* p) override;
};

struct assignment_stmt_t : stmt_t {
    std::string identifier;
    expr_t* rvalue;

    void undo(parser_t* p) override;
};

/* ---------------------- */

struct expr_t : undoable_t { };

struct arith_expr_t : expr_t {
    term_t* left;
    arithop_t* op;
    expr_t* right;

    void undo(parser_t* p) override;
};

struct rel_expr_t : expr_t {
    term_t* left;
    relop_t* op;
    expr_t* right;

    void undo(parser_t* p) override;

};

struct neg_expr_t : expr_t {
    term_t* value;

    void undo(parser_t* p) override;
};

struct term_expr_t : expr_t {
    term_t* t;

    void undo(parser_t* p) override;
};

struct term_t : undoable_t {
    bool is_literal;

    void undo(parser_t* p) override;
};

struct id_term_t : term_t {
    std::string identifier;

    id_term_t() { is_literal = false; }
};

struct lit_term_t : term_t {
    int literal;

    lit_term_t() { is_literal = true; }
};

struct arithop_t : undoable_t {
    void undo(parser_t* p) override;
};

struct arithop_plus_t : arithop_t { };

struct arithop_minus_t : arithop_t { };

struct relop_t :  undoable_t {
    void undo(parser_t* p) override;
};

struct relop_equals_t : relop_t { };

struct relop_not_equals_t : relop_t { };

#endif
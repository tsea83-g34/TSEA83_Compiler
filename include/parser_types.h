#ifndef COM_PARSER_TYPES_H
#define COM_PARSER_TYPES_H

#include <string>

#include "interfaces.h"
#include "translator.h"

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

// New binary operation system

struct binop_expr_t;

struct add_binop_t;
struct sub_binop_t;
struct eq_binop_t;
struct neq_binop_t;

/* ----------------------------- */
class parser_t;

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

struct param_decls_t : undoable_t, virtual printable_t, virtual translateable_t {
    param_decl_t* first;
    param_decls_t* rest;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
};

struct param_decl_t : undoable_t, virtual printable_t, virtual translateable_t {
    int type;
    std::string id;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
};

struct params_t : undoable_t, virtual printable_t, virtual translateable_t {
    expr_t* first;
    params_t* rest;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
};

/* ---------------------- */

/*       Statements       */

struct stmt_t : virtual undoable_t, virtual printable_t, virtual translateable_t { };

struct var_decl_t : decl_t, stmt_t {
    int type;
    std::string id;
    expr_t* value;

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

struct expr_t : undoable_t, printable_t, translateable_t {
    virtual bool evaluate(int* result) = 0;
};

struct arith_expr_t : expr_t {
    term_t* left;
    arithop_t* op;
    expr_t* right;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
    bool evaluate(int* result) override;
};

struct rel_expr_t : expr_t {
    term_t* left;
    relop_t* op;
    expr_t* right;

    void undo(parser_t* p) override;
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
    bool evaluate(int* result) override;
};

struct neg_expr_t : expr_t {
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

struct id_term_t : term_t {
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

struct lit_term_t : term_t {
    int literal;

    lit_term_t() { is_literal = true; }
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
    bool evaluate(int* result) override;
};

struct arithop_t : undoable_t, printable_t, translateable_t {
    void undo(parser_t* p) override;
    virtual bool evaluate(int* result, term_t* left, expr_t* right) = 0;
};

struct arithop_plus_t : arithop_t {
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
    bool evaluate(int* result, term_t* left, expr_t* right) override;
};

struct arithop_minus_t : arithop_t {
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
    bool evaluate(int* result, term_t* left, expr_t* right) override;
};

struct relop_t : undoable_t, printable_t, translateable_t {
    void undo(parser_t* p) override;
    virtual bool evaluate(int* result, term_t* left, expr_t* right) = 0;
};

struct relop_equals_t : relop_t {
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
    bool evaluate(int* result, term_t* left, expr_t* right) override;
};

struct relop_not_equals_t : relop_t {
    std::string get_string(parser_t* p) override;
    int translate(translator_t* t) override;
    bool evaluate(int* result, term_t* left, expr_t* right) override;
};

// New binary operation system

struct binop_expr_t : expr_t {
    term_t* first;
    expr_t* rest;
    bool left_assoc;

    void undo(parser_t* p) override;
    binop_expr_t() : left_assoc(false), first(nullptr), rest(nullptr) { }
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

// Conditional equality
struct eq_binop_t : binop_expr_t {
    eq_binop_t() : binop_expr_t() {}

    std::string get_string(parser_t* p) override;
    int translate(translator_t* p) override;
    bool evaluate(int* result) override;
    eq_binop_t* duplicate() override;
};

// Conditional non-equality
struct neq_binop_t : binop_expr_t {
    neq_binop_t() : binop_expr_t() {}

    std::string get_string(parser_t* p) override;
    int translate(translator_t* p) override;
    bool evaluate(int* result) override;
    neq_binop_t* duplicate() override;
};

#endif
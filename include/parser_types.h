#ifndef COM_PARSER_TYPES_H
#define COM_PARSER_TYPES_H

#include <string>

/* First tier C-- grammar

    e denotes empty string

    program     ->  decls 

    decls       ->  decl decls
                |   e

    decl        ->  func_decl
                |   var_decl
    
    var_decl    ->  type id
                |   type id "=" expr

    func_decl   ->  type id ( ) ;
                |   type id ( ) block_stmt

    stmt        ->  block_stmt
                |   if ( expr ) stmt
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
struct decl;
struct decls;
struct var_decl;
struct func_decl;

struct stmt;
struct stmts;
struct block_stmt;

struct if_stmt;
struct assignment_stmt;

struct expr;
struct arith_expr;
struct term;

struct arithop;
struct relop;
/* ----------------------------- */


/*      Declarations      */
struct decls {
    decl*   first;
    decls*  rest;
};

struct decl { };

struct var_decl : decl, stmt {
    int type;
    std::string id;
    std::string value;
};

struct func_decl : decl {
    int type;
    std::string id;
    block_stmt* stmt;
};

/* ---------------------- */

/*       Statements       */

struct stmt { };

struct stmts {
    stmt* first;
    stmts* rest;
};

struct block_stmt : stmt {
    stmts* statements;
};

struct if_stmt : stmt {
    expr* cond;
    stmt* actions;
};

struct assignment_stmt : stmt {
    std::string identifier;
    expr* rvalue;
};

/* ---------------------- */

struct expr { };

struct arith_expr : expr {
    term* left;
    arithop* op;
    expr* right;
};

struct rel_expr : expr {
    term* left;
    relop* op;
    expr* right;
};

struct neg_expr : expr {
    term* value;
};

struct term_expr : expr {
    term* t;
};

struct term { };

struct id_term : term {
    std::string identifier;
};

struct lit_term : term {
    int literal;
};

#endif
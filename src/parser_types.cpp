
#include "../include/parser_types.h"
#include "../include/parser.h"

void program_t::undo(parser_t* p) {
    decls->undo(p);
}

void decls_t::undo(parser_t* p) {
    
    if (rest != nullptr) rest->undo(p);
    first->undo(p);
}

void func_decl_t::undo(parser_t* p) {
    return;
}

void var_decl_t::undo(parser_t* p) {
    
    if (value != nullptr) {

        // Put back ; token
        p->put_back_token(tokens.back());
        tokens.pop_back();

        value->undo(p);
        // Put back assignment token
        p->put_back_token(tokens.back());
        tokens.pop_back();
    }

    // Put back id token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    // Put back type token
    p->put_back_token(tokens.back());
    tokens.pop_back();

}

void stmts_t::undo(parser_t* p) {

    if (rest != nullptr) rest->undo(p);
    first->undo(p);
}

void block_stmt_t::undo(parser_t* p) {
    
    // Put back } token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    if (statements != nullptr) statements->undo(p);

    // Put back { token
    p->put_back_token(tokens.back());
    tokens.pop_back();
}

void if_stmt_t::undo(parser_t* p) {
    
    if (actions != nullptr) actions->undo(p);

    // Put back ) token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    cond->undo(p);

    // Put back ( token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    // Put back if token
    p->put_back_token(tokens.back());
    tokens.pop_back();
}

void assignment_stmt_t::undo(parser_t* p) {
    
    // Put back ; token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    rvalue->undo(p);

    // Put back = token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    // Put back id token
    p->put_back_token(tokens.back());
    tokens.pop_back();
}

void arith_expr_t::undo(parser_t* p) {
    
    right->undo(p);
    op->undo(p);
    left->undo(p);
}

void rel_expr_t::undo(parser_t* p) {
    
    right->undo(p);
    op->undo(p);
    left->undo(p);

}

void neg_expr_t::undo(parser_t* p) {
    
    value->undo(p);
    
    // Put back - token
    p->put_back_token(tokens.back());
    tokens.pop_back();
}

void term_expr_t::undo(parser_t* p) {
    
    t->undo(p);

}

void term_t::undo(parser_t* p) {
    
    // Put back id or literal token
    p->put_back_token(tokens.back());
    tokens.pop_back();

}

void arithop_t::undo(parser_t* p) {
    
    // Put back + or - token
    p->put_back_token(tokens.back());
    tokens.pop_back();

}

void relop_t::undo(parser_t* p) {
    
    // Put back == or != token
    p->put_back_token(tokens.back());
    tokens.pop_back();

}

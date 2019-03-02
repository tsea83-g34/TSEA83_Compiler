
#include "../include/parser.h"



parser_t::parser_t(lex::lexer *l) : lexical_analyzer(l) {

    init_productions();

    type_map = std::unordered_map<std::string, int>();
    type_map.insert({"int", 0});

    token_queue = std::deque<lex::token*>();
    current_pos = 0;
}


void parser_t::init_productions() { }


lex::token* parser_t::get_token() {
    
    if (current_pos == token_queue.size()) {
        token_queue.push_back(lexical_analyzer->get_next_token());
    }
    return token_queue[current_pos++];
}

bool parser_t::is_type(const std::string& s) {
    return type_map.count(s);
}

// Construct specific matching functions

program_t* parser_t::match_program(parser_t *p) {
    
    program_t *program = new program_t();
    program->decls = (decls_t*) match_decls(p);
    return program;
}

decls_t* parser_t::match_decls(parser_t* p) {
    
    decls_t* ds;
    
    ds = match_decls_1(p);
    if (ds != nullptr) return ds;

    ds = match_decls_2(p);
    return ds;
}

decl_t* parser_t::match_decl(parser_t* p) {

    decl_t* d;
    
    d = match_decl_var(p);
    if (d != nullptr) return d;

    d = match_decl_func(p);
    return d;
}

func_decl_t* parser_t::match_decl_func(parser_t* p) {

    func_decl_t* d;

    d = match_decl_func_1(p);
    if (d != nullptr) return d;

    
    return nullptr;
}

var_decl_t* parser_t::match_decl_var(parser_t* p) {
    return nullptr;
}

stmts_t* parser_t::match_stmts(parser_t* p) {
    
    stmts_t* stmts;

    stmts = match_stmts_1(p);
    if (stmts != nullptr) return stmts;

    stmts = match_stmts_2(p);
    return stmts;
}

stmt_t* parser_t::match_stmt(parser_t* p) {
    
    stmt_t* stmt;

    stmt = match_stmt_assign(p);
    if (stmt != nullptr) return stmt;

    stmt = match_stmt_decl(p);
    if (stmt != nullptr) return stmt;

    stmt = match_stmt_block(p);
    if (stmt != nullptr) return stmt;

    stmt = match_stmt_if(p);
    return stmt;
}

expr_t* parser_t::match_expr(parser_t* p) {
    
    expr_t* expr;
    
    expr = match_expr_arithop(p);
    if (expr != nullptr) return expr;

    expr = match_expr_relop(p);
    if (expr != nullptr) return expr;

    expr = match_expr_term(p);
    if (expr != nullptr) return expr;

    expr = match_expr_negated(p);
    return expr;
}

arithop_t* parser_t::match_arithop(parser_t* p) {
    
    arithop_t* op;

    op = match_arithop_plus(p);
    if (op != nullptr) return op;

    op = match_arithop_minus(p);
    return op;
}

relop_t* parser_t::match_relop(parser_t* p) {
    
    relop_t* op;

    op = match_relop_equals(p);
    if (op != nullptr) return op;

    op = match_relop_not_equals(p);
    return op;
}

term_t* parser_t::match_term(parser_t* p) {
    
    term_t* term;
    
    term = match_term_identifier(p);
    if (term != nullptr) return term;

    term = match_term_literal(p);
    return term;
}

// Production specific matching functions

// decls -> decl decls
decls_t* parser_t::match_decls_1(parser_t* p) {
    
    decls_t* result = new decls_t;

    result->first = match_decl(p);

    // If could not find a declaration, free memory and return nullptr
    if (result->first == nullptr) {
        delete result;
        return nullptr;
    }

    // Otherwise, look for more declarations
    result->rest = (decls_t*) match_decls(p);

    return result;
}

// decls -> e
decls_t* parser_t::match_decls_2(parser_t* p) {
    // Empty production should just return nullptr, 
    // since empty production in struct means nullptr
    // and empty production always matches
    return nullptr;
}

// var_decl -> type id
var_decl_t* parser_t::match_decl_var_1(parser_t* p) {
    
    var_decl_t* d = new var_decl_t;

    lex::token *t;
    t = p->get_token();

    if (t->tag != lex::tag_t::ID || ((lex::id_token*) t)->lexeme) {
        delete d;
        return nullptr;
    }


}

// var_decl -> type id "=" expr
var_decl_t* parser_t::match_decl_var_2(parser_t* p) {
    return nullptr;
}

func_decl_t* parser_t::match_decl_func_1(parser_t* p) {
    return nullptr;
}

func_decl_t* parser_t::match_decl_func_2(parser_t* p) {
    return nullptr;
}

block_stmt_t* parser_t::match_stmt_block(parser_t* p) {
    return nullptr;
}

if_stmt_t* parser_t::match_stmt_if(parser_t* p) {
    return nullptr;
}

var_decl_t* parser_t::match_stmt_decl(parser_t* p) {
    return nullptr;
}

assignment_stmt_t* parser_t::match_stmt_assign(parser_t* p) {
    return nullptr;
}

stmts_t* parser_t::match_stmts_1(parser_t* p) {
    return nullptr;
}

stmts_t* parser_t::match_stmts_2(parser_t* p) {
    return nullptr;
}

arith_expr_t* parser_t::match_expr_arithop(parser_t* p) {
    return nullptr;
}

neg_expr_t* parser_t::match_expr_negated(parser_t* p) {
    return nullptr;
}

rel_expr_t* parser_t::match_expr_relop(parser_t* p) {
    return nullptr;
}

term_expr_t* parser_t::match_expr_term(parser_t* p) {
    return nullptr;
}

arithop_plus_t* parser_t::match_arithop_plus(parser_t* p) {
    return nullptr;
}

arithop_minus_t* parser_t::match_arithop_minus(parser_t* p) {
    return nullptr;
}

relop_equals_t* parser_t::match_relop_equals(parser_t* p) {
    return nullptr;
}

relop_not_equals_t* parser_t::match_relop_not_equals(parser_t* p) {
    return nullptr;
}

id_term_t* parser_t::match_term_identifier(parser_t* p) {
    return nullptr;
}

lit_term_t* parser_t::match_term_literal(parser_t* p) {
    return nullptr;
}
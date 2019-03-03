
#include "../include/parser.h"



parser_t::parser_t(lex::lexer *l) : lexical_analyzer(l) {

    init_productions();

    type_map = std::unordered_map<std::string, int>();
    type_map.insert({"int", 0});

    token_queue = std::deque<lex::token*>();
    current_pos = 0;
}


void parser_t::init_productions() { }


inline lex::token* parser_t::get_token() {
    
    if (token_queue.size()) {
        auto result = token_queue.front();
        token_queue.pop_front();
        return result;
    } else {
        return lexical_analyzer->get_next_token();
    }
}

inline void parser_t::put_back_token(lex::token* t) {
    token_queue.push_front(t);
}

inline bool parser_t::is_type(const std::string& s) {
    return type_map.count(s);
}

inline bool parser_t::is_type(const lex::token* t) {
    return t->tag == lex::tag_t::ID && is_type(static_cast<const lex::id_token*>(t)->lexeme);
}

inline int parser_t::get_type(const std::string& s) {
    return type_map[s];
}

inline int parser_t::get_type(const lex::id_token* t) {
    return type_map[t->lexeme];
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

    d = match_decl_func_2(p);
    return d;
}

var_decl_t* parser_t::match_decl_var(parser_t* p) {
    
    // This one is simple, since one of the productions of a variable declaration is the prefix of
    // the other. SO both have been implemented in the same function. There is a separate function
    // that checks the common suffix, and the one called here checks for the rest of the other
    // production
    return match_decl_var_2(p);
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

    lex::token *type_token;
    lex::token *id_token;
    // Get type
    type_token = p->get_token();

    // If first token is not a word token or is not a type, put back token, delete d and return nullptr
    if (!p->is_type(type_token)) {
        p->put_back_token(type_token);
        delete d;
        return nullptr;
    }

    // Read type from type map
    d->type = p->get_type(static_cast<lex::id_token*>(type_token));

    // Get identifier
    id_token = p->get_token();
    
    // If token is not an identifier, put back tokens in reverse order, delete d and return nullptr
    if (id_token->tag != lex::tag_t::ID) {
        p->put_back_token(id_token);
        p->put_back_token(type_token);
        delete d;
        return nullptr;
    }

    d->id = static_cast<lex::id_token*>(id_token)->lexeme;

    // If successful, return declaration and delete tokens
    delete type_token;
    delete id_token;
    return d;
}

// var_decl -> type id "=" expr
var_decl_t* parser_t::match_decl_var_2(parser_t* p) {

    // Use other function to try to find first part of the declaration
    var_decl_t* d  = match_decl_var_2(p);

    // If first part is not found, return nullptr
    if (d == nullptr) return nullptr;

    // Try to find assignment
    lex::token* equals = p->get_token();

    // If token is not an assignment operator put back token and return earlier declaration
    if (equals->tag != lex::tag_t::ASSIGNMENT) {
        p->put_back_token(equals);
        d->value = nullptr;
        return d;
    }

    // Else check for expression
    expr_t* value = match_expr(p);
    
    // If no expression is found, return nullptr
    // TODO: Throw exception instead and free memory
    if (value == nullptr) {
        delete d;
        return nullptr;
    }

    // If everything is in order, set value to found expr, free token and return d
    d->value = value;
    delete equals;
    return d;
}

// func_decl -> type id ( ) ;
func_decl_t* parser_t::match_decl_func_1(parser_t* p) {

    func_decl_t* d = new func_decl_t;

    lex::token* type_token;
    lex::token* id_token;
    lex::token* open_paren_token;
    lex::token* closed_paren_token;
    lex::token* semi_token;

    // Acquire first token
    type_token = p->get_token();

    // If first token is not a type token, put back token, delete d and return nullptr
    if (!p->is_type(type_token)) {
        p->put_back_token(type_token);
        delete d;
        return nullptr;
    }
    d->type = p->get_type(static_cast<lex::id_token*>(type_token));

    // Acquire identifier token
    id_token = p->get_token();
    // If the acquired token is not an identifier, put back tokens, delete d and return nullptr
    if (id_token->tag != lex::tag_t::ID) {
        p->put_back_token(id_token);
        p->put_back_token(type_token);
        delete d;
        return nullptr;
    }

    // Store function name
    d->id = static_cast<lex::id_token*>(id_token)->lexeme;

    // Acquire first parenthesis tokens
    open_paren_token    = p->get_token();
    closed_paren_token  = p->get_token();
    semi_token          = p->get_token();
    // If acquired tokens are not parenthesis
    if (open_paren_token->tag != lex::tag_t::OPEN_PAREN || closed_paren_token->tag != lex::tag_t::CLOSED_PAREN
            || semi_token->tag != lex::tag_t::SEMI_COLON) {
        p->put_back_token(semi_token);
        p->put_back_token(closed_paren_token);
        p->put_back_token(open_paren_token);
        p->put_back_token(id_token);
        p->put_back_token(type_token);
        delete d;
        return nullptr;
    }

    // If gotten so far, match is successful. Complete func_decl data structure and delete tokens
    d->stmt = nullptr;
    delete type_token;
    delete id_token;
    delete open_paren_token;
    delete closed_paren_token;
    delete semi_token;
    return d;
}

// func_decl -> type id ( ) block_stmt
func_decl_t* parser_t::match_decl_func_2(parser_t* p) {
    func_decl_t* d = new func_decl_t;

    lex::token* type_token;
    lex::token* id_token;
    lex::token* open_paren_token;
    lex::token* closed_paren_token;

    // Acquire first token
    type_token = p->get_token();

    // If first token is not a type token, put back token, delete d and return nullptr
    if (!p->is_type(type_token)) {
        p->put_back_token(type_token);
        delete d;
        return nullptr;
    }
    d->type = p->get_type(static_cast<lex::id_token*>(type_token));

    // Acquire identifier token
    id_token = p->get_token();
    // If the acquired token is not an identifier, put back tokens, delete d and return nullptr
    if (id_token->tag != lex::tag_t::ID) {
        p->put_back_token(id_token);
        p->put_back_token(type_token);
        delete d;
        return nullptr;
    }

    // Store function name
    d->id = static_cast<lex::id_token*>(id_token)->lexeme;

    // Acquire first parenthesis tokens
    open_paren_token    = p->get_token();
    closed_paren_token  = p->get_token();
    // If acquired tokens are not parenthesis
    if (open_paren_token->tag != lex::tag_t::OPEN_PAREN || closed_paren_token->tag != lex::tag_t::CLOSED_PAREN) {
        p->put_back_token(closed_paren_token);
        p->put_back_token(open_paren_token);
        p->put_back_token(id_token);
        p->put_back_token(type_token);
        delete d;
        return nullptr;
    }

    // Acquire block statement
    block_stmt_t* bs = match_stmt_block(p);
    
    // If unsuccessful in finding block statement, put back tokens, delete d and return nullptr
    if (bs == nullptr) { 
        p->put_back_token(closed_paren_token);
        p->put_back_token(open_paren_token);
        p->put_back_token(id_token);
        p->put_back_token(type_token);
        delete d;
        return nullptr;
    }

    // If gotten so far, match is successful. Complete func_decl data structure and delete tokens
    d->stmt = bs;
    delete type_token;
    delete id_token;
    delete open_paren_token;
    delete closed_paren_token;
    return d;
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
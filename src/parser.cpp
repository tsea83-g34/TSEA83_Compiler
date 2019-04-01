
#include "../include/parser.h"
#include "../include/parser_types.h"

#include <iostream>

parser_t::parser_t(lex::lexer *l) : lexical_analyzer(l) {

    type_map = std::unordered_map<std::string, int>();
    type_map.insert({"int", 0});

    type_name_map = std::unordered_map<int, std::string>();
    type_name_map.insert({0, "int"});

    token_queue = std::deque<lex::token*>();
    current_pos = 0;
}

program_t* parser_t::parse_token_stream() {
    return match_program(this);
}

inline lex::token* parser_t::get_token() {
    
    if (token_queue.size()) {
        auto result = token_queue.front();
        token_queue.pop_front();
        return result;
    } else {
        return lexical_analyzer->get_next_token();
    }
}

void parser_t::put_back_token(lex::token* t) {
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

std::string parser_t::get_type_name(int type) {
    return type_name_map[type];
}

// Construct specific matching functions

program_t* parser_t::match_program(parser_t *p) {
    
    std::cout << "Matching program" << std::endl;

    decls_t* d = match_decls(p);

    if (d == nullptr) {
        std::cout << "Failed matching decls" << std::endl;
        return nullptr;
    }
    
    program_t *program = new program_t();
    program->decls = d;
    return program;
}

decls_t* parser_t::match_decls(parser_t* p) {

    std::cout << "Matching decls" << std::endl;
    decls_t* ds;

    lex::token* test_token = p->get_token();
    lex::tag_t tag = test_token->tag;
    p->put_back_token(test_token);
    std::cout << "Decl test token tag: " << (int) tag << std::endl;
    if (tag == lex::tag_t::eof) return nullptr;
    
    ds = match_decls_1(p);
    if (ds != nullptr) return ds;

    ds = match_decls_2(p);
    return ds;
}

decl_t* parser_t::match_decl(parser_t* p) {
    std::cout << "Matching decl" << std::endl;
    decl_t* d;

    d = match_decl_var(p);
    if (d != nullptr) return d;

    d = match_decl_func(p);
    if (d != nullptr) return d;

    return nullptr;
}

// TODO
func_decl_t* parser_t::match_decl_func(parser_t* p) {

    func_decl_t* d;

    d = match_decl_func_1(p);
    if (d != nullptr) return d;

    d = match_decl_func_2(p);
    return d;
}

var_decl_t* parser_t::match_decl_var(parser_t* p) {
    
    // This one is simple, since one of the productions of a variable declaration is the prefix of
    // the other. So both have been implemented in the same function. There is a separate function
    // that checks the common suffix, and the one called here checks for the rest of the other
    // production
    var_decl_t* result = match_decl_var_2(p);
    
    // If failed to match declaration, return
    if (result == nullptr) return nullptr;
    
    // Acquire semi colon
    lex::token* semi_colon_token = p->get_token();

    if (semi_colon_token->tag != lex::tag_t::SEMI_COLON) {
        p->put_back_token(semi_colon_token);
        result->undo(p);
        delete result;
        return nullptr;
    }

    return result;
}

stmts_t* parser_t::match_stmts(parser_t* p) {
    
    stmts_t* stmts;
    std::cout << "Matching statement list" << std::endl;
    stmts = match_stmts_1(p);
    if (stmts != nullptr) return stmts;

    stmts = match_stmts_2(p);
    return stmts;
}

stmt_t* parser_t::match_stmt(parser_t* p) {
    
    std::cout << "Matching statement" << std::endl;
    stmt_t* stmt;

    stmt = match_stmt_assign(p);
    if (stmt != nullptr) return stmt;

    stmt = match_stmt_decl(p);
    if (stmt != nullptr) return stmt;

    std::cout << "beep" << std::endl;

    stmt = match_stmt_block(p);
    if (stmt != nullptr) return stmt;

    stmt = match_stmt_if(p);
    return stmt;
}

expr_t* parser_t::match_expr(parser_t* p) {
    
    expr_t* expr;

    std::cout << "Matching expression..." << std::endl;
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
    
    std::cout << "Matching decls_1" << std::endl;

    decl_t* first = match_decl(p);

    // If could not find a declaration, free memory and return nullptr
    if (first == nullptr) {
        return nullptr;
    }
    decls_t* result = new decls_t;
    result->first = first;

    // Otherwise, look for more declarations
    decls_t* rest = (decls_t*) match_decls(p);
    result->rest = rest;
    std::string s((rest == nullptr) ? "rest not found" : "rest found");
    std::cout << s << std::endl; 


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

    lex::token *type_token;
    lex::token *id_token;
    // Get type
    type_token = p->get_token();

    // If first token is not a word token or is not a type, put back token, delete d and return nullptr
    if (!p->is_type(type_token)) {
        p->put_back_token(type_token);
        return nullptr;
    }

    // Get identifier
    id_token = p->get_token();
    
    // If token is not an identifier, put back tokens in reverse order, delete d and return nullptr
    if (id_token->tag != lex::tag_t::ID) {
        p->put_back_token(id_token);
        p->put_back_token(type_token);
        return nullptr;
    }

    // If successful, build syntax type
    var_decl_t* d = new var_decl_t();
    
    d->type = p->get_type(static_cast<lex::id_token*>(type_token));
    d->id = static_cast<lex::id_token*>(id_token)->lexeme;

    d->tokens.push_back(type_token);
    d->tokens.push_back(id_token);

    return d;
}

// var_decl -> type id "=" expr
var_decl_t* parser_t::match_decl_var_2(parser_t* p) {

    // Use other function to try to find first part of the declaration
    var_decl_t* d  = match_decl_var_1(p);
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

    std::cout << "assignment declaration" << std::endl;

    // Else check for expression
    expr_t* value = match_expr(p);
    
    // If no expression is found, return nullptr
    // TODO: Throw exception instead and free memory
    if (value == nullptr) {
        std::cout << "Failed to match rvalue expression" << std::endl;
        
        value->undo(p);
        p->put_back_token(equals);
        d->undo(p);
        delete value;
        delete d;
        return nullptr;
    }

    // If everything is in order, build syntax object
    // TODO: Add variable name to symbol table
    d->tokens.push_back(equals);
    
    d->value = value;

    return d;
}

// func_decl -> type id ( ) ;
func_decl_t* parser_t::match_decl_func_1(parser_t* p) {

    std::cout << "Matching func declaration" << std::endl;

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
        return nullptr;
    }

    // Acquire identifier token
    id_token = p->get_token();
    // If the acquired token is not an identifier, put back tokens, delete d and return nullptr
    if (id_token->tag != lex::tag_t::ID) {
        p->put_back_token(id_token);
        p->put_back_token(type_token);
        return nullptr;
    }

    // Store function name

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
        return nullptr;
    }

    // If gotten so far, match is successful.
    std::cout << "Function declaration match successful" << std::endl;
    // Build syntax object
    func_decl_t* result = new func_decl_t();
    result->type = p->get_type(static_cast<lex::id_token*>(type_token));
    result->id = static_cast<lex::id_token*>(id_token)->lexeme;
    result->stmt = nullptr;

    // Save tokens in syntax object
    result->tokens.push_back(type_token);
    result->tokens.push_back(id_token);
    result->tokens.push_back(open_paren_token);
    result->tokens.push_back(closed_paren_token);
    result->tokens.push_back(semi_token);

    return result;
}

// func_decl -> type id ( ) block_stmt
func_decl_t* parser_t::match_decl_func_2(parser_t* p) {

    std::cout << "Matching func definition" << std::endl;

    lex::token* type_token;
    lex::token* id_token;
    lex::token* open_paren_token;
    lex::token* closed_paren_token;

    // Acquire first token
    type_token = p->get_token();

    // If first token is not a type token, put back token, delete d and return nullptr
    if (!p->is_type(type_token)) {
        p->put_back_token(type_token);
        return nullptr;
    }

    // Acquire identifier token
    id_token = p->get_token();
    // If the acquired token is not an identifier, put back tokens, delete d and return nullptr
    if (id_token->tag != lex::tag_t::ID) {
        p->put_back_token(id_token);
        p->put_back_token(type_token);
        return nullptr;
    }

    // Acquire first parenthesis tokens
    open_paren_token    = p->get_token();
    closed_paren_token  = p->get_token();
    // If acquired tokens are not parenthesis
    if (open_paren_token->tag != lex::tag_t::OPEN_PAREN || closed_paren_token->tag != lex::tag_t::CLOSED_PAREN) {
        p->put_back_token(closed_paren_token);
        p->put_back_token(open_paren_token);
        p->put_back_token(id_token);
        p->put_back_token(type_token);
        return nullptr;
    }


    // Acquire block statement
    block_stmt_t* bs = match_stmt_block(p);
    
    // If unsuccessful in finding block statement, put back tokens and return nullptr
    if (bs == nullptr) {
        p->put_back_token(closed_paren_token);
        p->put_back_token(open_paren_token);
        p->put_back_token(id_token);
        p->put_back_token(type_token);
        return nullptr;
    }

    // If gotten so far, match is successful
    std::cout << "Function definition match successful" << std::endl;
    // Build syntax object
    func_decl_t* result = new func_decl_t();
    result->type = p->get_type(static_cast<lex::id_token*>(type_token));
    result->id = static_cast<lex::id_token*>(id_token)->lexeme;
    result->stmt = bs;

    // Store tokens in syntax object
    result->tokens.push_back(type_token);
    result->tokens.push_back(id_token);
    result->tokens.push_back(open_paren_token);
    result->tokens.push_back(closed_paren_token);

    return result;
}

// block_stmt  ->  { stmts }
block_stmt_t* parser_t::match_stmt_block(parser_t* p) {
    
    lex::token* open_brace;
    lex::token* closed_brace;

    // Acquire first token
    open_brace = p->get_token();

    // If first token is not an open brace, revert
    if (open_brace->tag != lex::tag_t::OPEN_BRACE) {
        p->put_back_token(open_brace);
        return nullptr;
    }
    std::cout << "Matched first brace" << std::endl;

    stmts_t* inner = match_stmts(p);

    // If unsuccessful in finding inner statements, revert
    if (inner == nullptr) {
        p->put_back_token(open_brace);
        return nullptr;
    }

    std::cout << "Matched inner statements" << std::endl;

    // Get token for closed brace
    closed_brace = p->get_token();
    if (closed_brace->tag != lex::tag_t::CLOSED_BRACE) {
        std::cout << "Debug" << std::endl;
        
        p->put_back_token(closed_brace);
        inner->undo(p);
        delete inner;
        p->put_back_token(open_brace);

        return nullptr;
    }

    std::cout << "Matched second brace" << std::endl;

    // If we got this far, we have a successful match

    // Build syntax object
    block_stmt_t* result = new block_stmt_t();
    result->statements = inner;

    // Store tokens
    result->tokens.push_back(open_brace);
    result->tokens.push_back(closed_brace);

    return result;
}

// stmt -> if ( expr ) stmt
if_stmt_t* parser_t::match_stmt_if(parser_t* p) {

    std::cout << "Matching if statement" << std::endl;

    lex::token* if_token;
    lex::token* open_paren_token;
    lex::token* closed_paren_token;

    if_token = p->get_token();
    open_paren_token = p->get_token();
    
    // Mismatching tokens, revert
    if (if_token->tag != lex::tag_t::IF || open_paren_token->tag != lex::tag_t::OPEN_PAREN) {
        p->put_back_token(open_paren_token);
        p->put_back_token(if_token);
        return nullptr;
    }

    // Acquire conditional expression
    expr_t* cond = match_expr(p);

    // If could not acquire conditional, revert
    if (cond == nullptr) {
        p->put_back_token(open_paren_token);
        p->put_back_token(if_token);
        return nullptr; 
    }

    closed_paren_token = p->get_token();

    if (closed_paren_token->tag != lex::tag_t::CLOSED_PAREN) {
        p->put_back_token(closed_paren_token);
        cond->undo(p);
        p->put_back_token(open_paren_token);
        p->put_back_token(if_token);

        delete cond;
        return nullptr;
    }

    stmt_t* stmt = match_stmt(p);

    if (stmt == nullptr) {
        p->put_back_token(closed_paren_token);
        cond->undo(p);
        p->put_back_token(open_paren_token);
        p->put_back_token(if_token);
        delete cond;
        return nullptr;
    }

    // If we got this far, we have a successful match, delete tokens and return result
    
    // Build syntax object
    if_stmt_t* result = new if_stmt_t();
    result->cond = cond;
    result->actions = stmt;

    // Store tokens
    result->tokens.push_back(if_token);
    result->tokens.push_back(open_paren_token);
    result->tokens.push_back(closed_paren_token);

    return result;
    std::cout << "Finished matching if statement" << std::endl;
}

// stmt -> var_decl ; 
var_decl_t* parser_t::match_stmt_decl(parser_t* p) {
    
    var_decl_t* result = match_decl_var(p);

    // If could not match a variable declaration, return nullptr
    if (result == nullptr) {
        return nullptr;
    }
    return result;
}

// stmt -> id "=" expr ;
assignment_stmt_t* parser_t::match_stmt_assign(parser_t* p) {

    lex::token* id_token;
    lex::token* assign_token;
    lex::token* semi_colon_token;

    id_token     = p->get_token();
    assign_token = p->get_token();

    // If first two tokens don't match, revert
    if (id_token->tag != lex::tag_t::ID || assign_token->tag != lex::tag_t::ASSIGNMENT) {
        p->put_back_token(assign_token);
        p->put_back_token(id_token);
        return nullptr;
    }

    // Acquire identifier from token
    expr_t* rvalue = match_expr(p);

    // If could not match expression, revert
    if (rvalue == nullptr) {
        p->put_back_token(assign_token);
        p->put_back_token(id_token);
        return nullptr;
    }

    semi_colon_token = p->get_token();

    if (semi_colon_token->tag != lex::tag_t::SEMI_COLON) {
        p->put_back_token(semi_colon_token);
        rvalue->undo(p);
        p->put_back_token(assign_token);
        p->put_back_token(id_token);
        delete rvalue;
        return nullptr;
    }
    
    // If gotten this far, match was successful

    // Build syntax object
    assignment_stmt_t* result = new assignment_stmt_t();
    result->identifier = static_cast<lex::id_token*>(id_token)->lexeme;
    result->rvalue = rvalue;

    // Store tokens
    result->tokens.push_back(id_token);
    result->tokens.push_back(assign_token);
    result->tokens.push_back(semi_colon_token);
    
    return result;
}

// stmts -> stmt stmts
stmts_t* parser_t::match_stmts_1(parser_t* p) {

    stmt_t* stmt = match_stmt(p);

    // If matching a single statement fails, return nullptr
    // A list of statements following this production must include at least one
    if (stmt == nullptr) {
        return nullptr;
    }

    stmts_t* result = new stmts_t();
    result->first = stmt;

    // Try matching more statements
    result->rest = match_stmts(p);
    
    return result;
}

// Maybe delete this function?
// stmts -> e
stmts_t* parser_t::match_stmts_2(parser_t* p) {
    return nullptr;
}

// expr -> term arithop expr
arith_expr_t* parser_t::match_expr_arithop(parser_t* p) {

    term_t* left = match_term(p);

    if (left == nullptr) {
        std::cout << "term not found" << std::endl;
        return nullptr;
    }
    std::cout << "term found" << std::endl;

    arithop_t* op = match_arithop(p);

    if (op == nullptr) {
        std::cout << "operator not found" << std::endl;
        left->undo(p);
        delete left;
        return nullptr;
    }

    std::cout << "operator found" << std::endl;

    expr_t* right = match_expr(p);

    if (right == nullptr) {
        std::cout << "expr not found" << std::endl;
        op->undo(p);
        left->undo(p);
        
        delete op;
        delete left;
    }
    std::cout << "expr found" << std::endl;

    arith_expr_t* result = new arith_expr_t();

    result->left  = left;
    result->op    = op;
    result->right = right;
    
    return result;
}

// expr -> "-" term
neg_expr_t* parser_t::match_expr_negated(parser_t* p) {
    
    lex::token* neg_token  = p->get_token();

    // If first token is not a minus operator, or the second token is not a literal nor an identifier, revert
    if (neg_token->tag != lex::tag_t::MINUS ) {
        p->put_back_token(neg_token);
        return nullptr;
    }

    term_t* value = match_term(p);
    
    if (value == nullptr) {
        p->put_back_token(neg_token);
        return nullptr;
    }

    // If gotten this far, match is successful

    // Create syntax object
    neg_expr_t* result = new neg_expr_t();
    result->value = value;

    // Store token
    result->tokens.push_back(neg_token);
    
    return result;
}

// expr -> term relop expr
rel_expr_t* parser_t::match_expr_relop(parser_t* p) {

    term_t* left = match_term(p);

    if (left == nullptr) {
        return nullptr;
    }

    relop_t* op = match_relop(p);

    if (op == nullptr) {
        left->undo(p);
        delete left;
        return nullptr;
    }

    expr_t* right = match_expr(p);

    if (right == nullptr) {
        op->undo(p);
        left->undo(p);
        delete left;
        delete op;
        return nullptr;
    }

    // If gotten this far, match was successful

    // Build syntax object
    rel_expr_t* result = new rel_expr_t();
    result->left  = left;
    result->op    = op;
    result->right = right;

    return result;
}

// expr -> term
term_expr_t* parser_t::match_expr_term(parser_t* p) {
    

    term_t* t = match_term(p);

    if (t == nullptr) {
        return nullptr;
    }

    // If gotten this far, match was successful

    // Build syntax object
    term_expr_t* result = new term_expr_t();
    result->t = t;
    return result;
}

// arithop -> "+"
arithop_plus_t* parser_t::match_arithop_plus(parser_t* p) {

    lex::token* plus_token = p->get_token();

    if (plus_token->tag != lex::tag_t::PLUS) {
        p->put_back_token(plus_token);
        return nullptr;
    }

    // If gotten this far, match was successful

    // Build syntax object
    arithop_plus_t* result = new arithop_plus_t();

    // Store token
    result->tokens.push_back(plus_token);
    
    return result;
}

// arithop -> "-"
arithop_minus_t* parser_t::match_arithop_minus(parser_t* p) {

    lex::token* minus_token = p->get_token();

    if (minus_token->tag != lex::tag_t::MINUS) {
        p->put_back_token(minus_token);
        return nullptr;
    }

    // If gotten this far, match was successful

    // Build syntax object
    arithop_minus_t* result = new arithop_minus_t();

    // Store token
    result->tokens.push_back(minus_token);
    
    return result;
}

// relop -> "=="
relop_equals_t* parser_t::match_relop_equals(parser_t* p) {

    lex::token* equals_token = p->get_token();

    if (equals_token->tag != lex::tag_t::EQUALS) {
        p->put_back_token(equals_token);
        return nullptr;
    }

    // If gotten this far, match was successful

    // Build syntax object
    relop_equals_t* result = new relop_equals_t();

    // Store token
    result->tokens.push_back(equals_token);
    
    return result;
}

// relop -> "!="
relop_not_equals_t* parser_t::match_relop_not_equals(parser_t* p) {
    
    lex::token* not_equals_token = p->get_token();

    if (not_equals_token->tag != lex::tag_t::NOT_EQUALS) {
        p->put_back_token(not_equals_token);
        return nullptr;
    }

    // If gotten this far, match was successful

    // Build syntax object
    relop_not_equals_t* result = new relop_not_equals_t();

    // Store token
    result->tokens.push_back(not_equals_token);
    
    return result;
}

// term -> id
id_term_t* parser_t::match_term_identifier(parser_t* p) {
    
    lex::token* id_token = p->get_token();

    if (id_token->tag != lex::tag_t::ID) {
        p->put_back_token(id_token);
        return nullptr;
    }
    
    // If gotten this far, match was successful
    
    // Build syntax object
    id_term_t* result = new id_term_t();
    result->identifier = static_cast<lex::id_token*>(id_token)->lexeme;
    
    // Store token
    result->tokens.push_back(id_token);

    return result;
}

// term -> literal
lit_term_t* parser_t::match_term_literal(parser_t* p) {
    
    lex::token* literal_token = p->get_token();

    if (literal_token->tag != lex::tag_t::INT_LITERAL) {
        p->put_back_token(literal_token);
        return nullptr;
    }

    // If gotten this far, match was successful
    
    // Build syntax object
    lit_term_t* result = new lit_term_t();
    result->literal = static_cast<lex::int_literal_token*>(literal_token)->value;
    
    // Store token
    result->tokens.push_back(literal_token);

    return result;
}

#include "../include/parser.h"
#include "../include/parser_types.h"

#include <iostream>

parser_t::parser_t(lex::lexer *l) : lexical_analyzer(l) {

    type_map = std::unordered_map<std::string, int>();
    type_map.insert({"int", 0});
    type_map.insert({"char", 1});
    type_map.insert({"long", 2});

    type_name_map = std::unordered_map<int, std::string>();
    type_name_map.insert({0, "int"});

    token_queue = std::deque<lex::token*>();
    current_pos = 0;
}

parser_t::~parser_t() {

    while (!token_queue.empty()) {
        delete token_queue.back();
        token_queue.pop_back();
    }
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

const lex::token* parser_t::peek() {
    
    if (!token_queue.size()) {
        token_queue.push_front(get_token());
    }
    
    return token_queue.front();
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
    decls_t* ds = nullptr;

    lex::token* test_token = p->get_token();
    lex::tag_t tag = test_token->tag;
    p->put_back_token(test_token);
    std::cout << "Decl test token tag: " << (int) tag << std::endl;
    
    if (tag == lex::tag_t::eof) return nullptr;
    
    // Try matching decl, if it fails, check if next token is EOF, if it is, everything is in order
    // If it is not rethrow the syntax error
    try {
        ds = match_decls_1(p);
    } catch (syntax_error e) {

        std::cout << "Caught syntax error in match_decls" << std::endl;
        if (p->peek()->tag != lex::tag_t::eof) {
            throw e;
        }
    }

    if (ds != nullptr) return ds;

    return match_decls_2(p);
}

decl_t* parser_t::match_decl(parser_t* p) {
    std::cout << "Matching decl" << std::endl;
    decl_t* d;

    d = match_decl_var(p);
    if (d != nullptr) return d;

    d = match_decl_func(p);
    if (d != nullptr) return d;

    throw syntax_error("Could not match declaration. Unexpected " + lex::token_names[(int) p->token_queue.front()->tag] + " token");
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
    // the other. So both have been implemented in the same function. There is a separate function
    // that checks the common suffix, and the one called here checks for the rest of the other
    // production
    var_decl_t* result = match_decl_var_2(p);
    
    // If failed to match declaration, return
    if (result == nullptr) return nullptr;
    
    // Acquire semi colon
    lex::token* semi_colon_token = p->get_token();

    if (semi_colon_token->tag != lex::tag_t::SEMI_COLON) {
        std::cout << "Failed acquiring semi colon: " << (int) semi_colon_token->tag << std::endl;

        p->put_back_token(semi_colon_token);
        result->undo(p);
        delete result;
        return nullptr;
    }

    result->tokens.push_back(semi_colon_token);
    return result;
}

// param_decls -> param params
//             |  e  
param_decls_t* parser_t::match_param_decls(parser_t* p) {

    std::cout << "Matching param declarations" << std::endl;
    param_decl_t* first = match_param_decl(p);
    // If failed matching first, return nullptr
    if (first == nullptr) {
        std::cout << "Failed matching param declarations" << std::endl;
        return nullptr;
    }

    param_decls_t* result = new param_decls_t();
    result->first = first;

    // TODO add syntax error handling here

    // Try matching more decls
    result->rest = match_param_decls(p);
    
    return result;
}

// param_decl -> type id
param_decl_t* parser_t::match_param_decl(parser_t* p) {

    lex::token* type_token;
    lex::token* id_token;

    type_token  = p->get_token();
    id_token    = p->get_token();

    if (!p->is_type(type_token) || id_token->tag != lex::tag_t::ID) {
        p->put_back_token(id_token);
        p->put_back_token(type_token);
        return nullptr;
    }

    // If gotten this far, match was succesful

    // Create syntax object
    param_decl_t* result = new param_decl_t();
    result->type    = p->get_type(static_cast<lex::id_token*>(type_token));
    result->id      = static_cast<lex::id_token*>(id_token)->lexeme;

    // Store token
    result->tokens.push_back(type_token);
    result->tokens.push_back(id_token);

    return result;
}

//  params      ->  expr params
//              |   e
params_t* parser_t::match_params(parser_t* p) {

    expr_t* first;

    try {
        first = match_expr(p);
    } catch (syntax_error e) {
        return nullptr;
    }
    
    // Change associativity of expression if needed
    first = binop_expr_t::rewrite(first);
    

    params_t* result = new params_t();
    result->first = first;

    // Try matching more parameters
    result->rest = match_params(p);

    return result;
}

stmts_t* parser_t::match_stmts(parser_t* p) {
    
    stmts_t* stmts = nullptr;
    std::cout << "Matching statement list" << std::endl;

    try {
        stmts = match_stmts_1(p);
    } catch (syntax_error e) {

        std::cout << "Caught syntax error in match_stmts" << std::endl;
        if (p->peek()->tag != lex::tag_t::CLOSED_BRACE) {
            throw e;
        }
    }
    if (stmts != nullptr) return stmts;

    return match_stmts_2(p);
}

stmt_t* parser_t::match_stmt(parser_t* p) {
    
    std::cout << "Matching statement" << std::endl;
    stmt_t* stmt;

    stmt = match_stmt_assign(p);
    if (stmt != nullptr) return stmt;

    stmt = match_stmt_decl(p);
    if (stmt != nullptr) return stmt;

    stmt = match_stmt_block(p);
    if (stmt != nullptr) return stmt;

    stmt = match_stmt_return(p);
    if (stmt != nullptr) return stmt;

    stmt = match_stmt_expr(p);
    if (stmt != nullptr) return stmt;

    stmt = match_stmt_if(p);
    if (stmt != nullptr) return stmt;

    stmt = match_stmt_while(p);
    if (stmt != nullptr) return stmt;

    throw syntax_error("Could not match statement. Unexpected " + lex::token_names[(int) p->token_queue.front()->tag] + " token");
}

expr_t* parser_t::match_expr(parser_t* p) {
    
    expr_t* expr;

    std::cout << "Matching expression..." << std::endl;
    expr = match_expr_binop(p);
    if (expr != nullptr) return expr;

    expr = match_expr_term(p);
    if (expr != nullptr) return expr;

    expr = match_expr_negated(p);
    if (expr != nullptr) return expr;

    throw syntax_error("Could not match expression. Unexpected " + lex::token_names[(int) p->token_queue.front()->tag] + " token");
}

binop_expr_t* parser_t::match_binop(parser_t* p) {
   
    lex::token* op_token = p->get_token();
    binop_expr_t* result = nullptr; 

    switch (op_token->tag) {
        case lex::tag_t::PLUS:
            result = new add_binop_t();
            break;
        case lex::tag_t::MINUS:
            result = new sub_binop_t();
            break;
        case lex::tag_t::EQUALS:
            result = new eq_binop_t();
            break;
        case lex::tag_t::NOT_EQUALS:
            result = new neq_binop_t();
            break;
        default:
            p->put_back_token(op_token);
            return nullptr;
    }

    // If gotten this far, match was successful
    
    // Store token
    result->tokens.push_back(op_token);

    return result;
}

term_t* parser_t::match_term(parser_t* p) {
    
    term_t* term;

    term = match_term_call(p);
    if (term != nullptr) return term;
    
    term = match_term_identifier(p);
    if (term != nullptr) return term;

    term = match_term_literal(p);
    if (term != nullptr) return term;

    term = match_term_expr(p);
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

    // Change associativity of expression if needed
    value = binop_expr_t::rewrite(value);

    // TODO: Add variable name to symbol table
    d->tokens.push_back(equals);
    
    d->value = value;

    return d;
}

// func_decl -> type id ( param_decls ) ;
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

    // Acquire first parenthesis tokens
    open_paren_token    = p->get_token();

    if (open_paren_token->tag != lex::tag_t::OPEN_PAREN) {
        p->put_back_token(open_paren_token);
        p->put_back_token(id_token);
        p->put_back_token(type_token);
        return nullptr;
    }

    // Try matching parameter declarations, if none is found, it's not a problem
    param_decls_t* param_list = match_param_decls(p);


    closed_paren_token  = p->get_token();
    semi_token          = p->get_token();
    // If acquired tokens are not parenthesis
    if (closed_paren_token->tag != lex::tag_t::CLOSED_PAREN || semi_token->tag != lex::tag_t::SEMI_COLON) {
        
        p->put_back_token(semi_token);
        p->put_back_token(closed_paren_token);

        if (param_list != nullptr) {
            param_list->undo(p);
            delete param_list;
        }

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
    result->param_list = param_list;

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
        std::cout << "Could not match function type: " << (int) type_token->tag << " " << static_cast<lex::id_token*>(type_token)->lexeme << std::endl;
        p->put_back_token(type_token);
        return nullptr;
    }

    // Acquire identifier token
    id_token = p->get_token();
    // If the acquired token is not an identifier, put back tokens, delete d and return nullptr
    if (id_token->tag != lex::tag_t::ID) {
        std::cout << "Could not match function identifier" << std::endl;
        p->put_back_token(id_token);
        p->put_back_token(type_token);
        return nullptr;
    }

    // Acquire first parenthesis tokens
    open_paren_token = p->get_token();

    if (open_paren_token->tag != lex::tag_t::OPEN_PAREN) {
        p->put_back_token(open_paren_token);
        p->put_back_token(id_token);
        p->put_back_token(type_token);
        return nullptr;
    }

    // Try matching parameter declarations, if none is found, it's not a problem
    param_decls_t* param_list = match_param_decls(p);

    closed_paren_token  = p->get_token();
    // If acquired tokens are not parenthesis
    if (closed_paren_token->tag != lex::tag_t::CLOSED_PAREN) {
        std::cout << "Could not match function parenthesis" << std::endl;
        p->put_back_token(closed_paren_token);

        // If there is a parameter list, undo it
        if (param_list != nullptr) {
            param_list->undo(p);
            delete param_list;
        }

        p->put_back_token(open_paren_token);
        p->put_back_token(id_token);
        p->put_back_token(type_token);
        return nullptr;
    }

    // Acquire block statement
    block_stmt_t* bs = match_stmt_block(p);
    
    // If unsuccessful in finding block statement, put back tokens and return nullptr
    if (bs == nullptr) {
        std::cout << "Could not match function block statement" << std::endl;
        p->put_back_token(closed_paren_token);

        // If there is a parameter list, undo it
        if (param_list != nullptr) {
            param_list->undo(p);
            delete param_list;
        }

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
    result->param_list = param_list;

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
    // Not matching statements is okay
    /*if (inner == nullptr) {
        p->put_back_token(open_brace);
        return nullptr;
    }*/

    std::cout << "Matched inner statements" << std::endl;

    // Get token for closed brace
    closed_brace = p->get_token();
    if (closed_brace->tag != lex::tag_t::CLOSED_BRACE) {
        std::cout << "Debug" << std::endl;
        
        p->put_back_token(closed_brace);
        inner->undo(p);
        p->put_back_token(open_brace);

        delete inner;
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
    expr_t* cond;

    try {
        cond = match_expr(p);
    } catch (syntax_error e) {

        // If could not acquire conditional, revert
        p->put_back_token(open_paren_token);
        p->put_back_token(if_token);
        throw syntax_error("Could not match conditional expression for if statement");
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
    
    // Change associativity of expression if needed
    cond = binop_expr_t::rewrite(cond);
    

    // Build syntax object
    if_stmt_t* result = new if_stmt_t();
    result->cond = cond;
    result->actions = stmt;

    // Store tokens
    result->tokens.push_back(if_token);
    result->tokens.push_back(open_paren_token);
    result->tokens.push_back(closed_paren_token);

    std::cout << "Finished matching if statement" << std::endl;
    return result;
}

// stmt -> while ( expr ) stmt
while_stmt_t* parser_t::match_stmt_while(parser_t* p) {

    std::cout << "Matching while statement" << std::endl;

    lex::token* while_token;
    lex::token* open_paren_token;
    lex::token* closed_paren_token;

    while_token = p->get_token();
    open_paren_token = p->get_token();
    
    // Mismatching tokens, revert
    if (while_token->tag != lex::tag_t::WHILE || open_paren_token->tag != lex::tag_t::OPEN_PAREN) {
        p->put_back_token(open_paren_token);
        p->put_back_token(while_token);
        return nullptr;
    }

    // Acquire conditional expression
    expr_t* cond;

    try {
        cond = match_expr(p);
    } catch (syntax_error e) {

        // If could not acquire conditional, revert
        p->put_back_token(open_paren_token);
        p->put_back_token(while_token);
        throw syntax_error("Could not match conditional expression for if statement");
    }

    closed_paren_token = p->get_token();

    if (closed_paren_token->tag != lex::tag_t::CLOSED_PAREN) {
        p->put_back_token(closed_paren_token);
        cond->undo(p);
        p->put_back_token(open_paren_token);
        p->put_back_token(while_token);

        delete cond;
        return nullptr;
    }

    stmt_t* stmt = match_stmt(p);

    if (stmt == nullptr) {
        p->put_back_token(closed_paren_token);
        cond->undo(p);
        p->put_back_token(open_paren_token);
        p->put_back_token(while_token);
        delete cond;
        return nullptr;
    }

    // If we got this far, we have a successful match, delete tokens and return result
    
    // Change associativity of expression if needed
    cond = binop_expr_t::rewrite(cond);
    

    // Build syntax object
    while_stmt_t* result = new while_stmt_t();
    result->cond = cond;
    result->actions = stmt;

    // Store tokens
    result->tokens.push_back(while_token);
    result->tokens.push_back(open_paren_token);
    result->tokens.push_back(closed_paren_token);

    std::cout << "Finished matching while statement" << std::endl;
    return result;
}

// stmt -> return expr ;
return_stmt_t* parser_t::match_stmt_return(parser_t* p) {

    lex::token* return_token;
    lex::token* semi_colon_token;

    return_token = p->get_token();

    if (return_token->tag != lex::tag_t::RETURN) {
        p->put_back_token(return_token);
        return nullptr;
    }
    
    expr_t* return_value;

    // Try matching return value expression
    try {
        return_value = match_expr(p);
    } catch (syntax_error e) {
        p->put_back_token(return_token);
        throw syntax_error("Could not match expression for return statement");
    }

    semi_colon_token = p->get_token();

    if (semi_colon_token->tag != lex::tag_t::SEMI_COLON) {
        
        p->put_back_token(semi_colon_token);
        return_value->undo(p);
        p->put_back_token(return_token);

        delete return_value;
        return nullptr;
    }

    // If gotten so far, match is successful


    // Change associativity of expression if needed
    return_value = binop_expr_t::rewrite(return_value);
    
    
    // Build syntax object
    return_stmt_t* result = new return_stmt_t();
    result->return_value = return_value;

    // Store tokens
    result->tokens.push_back(return_token);
    result->tokens.push_back(semi_colon_token);

    return result;
}

// stmt -> expr ;
expr_stmt_t* parser_t::match_stmt_expr(parser_t* p) {

    lex::token* semi_colon_token;

    expr_t* e = nullptr;
    try {
        e = match_expr(p);
    } catch (syntax_error e) {

        // If could not match expression, return nullptr
        return nullptr;
    }

    semi_colon_token = p->get_token();

    if (semi_colon_token->tag != lex::tag_t::SEMI_COLON) {
        p->put_back_token(semi_colon_token);
        e->undo(p);
        delete e;
        return nullptr;
    }

    // If gotten this far, match was successful
    
    // Change associativity of expression if needed
    e = binop_expr_t::rewrite(e);
    

    // Build syntax object
    expr_stmt_t* result = new expr_stmt_t();
    result->e = e;

    // Save token
    result->tokens.push_back(semi_colon_token);

    return result;
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
    expr_t* rvalue;

    try {
        rvalue = match_expr(p);
    } catch (syntax_error e) {
        // If could not match expression, revert
        std::cout << "Could not match assignment expr" << std::endl;
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


    // Change associativity of expression if needed
    rvalue = binop_expr_t::rewrite(rvalue);
    

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

binop_expr_t* parser_t::match_expr_binop(parser_t* p) {

    term_t* left = match_term(p);

    if (left == nullptr) {
        return nullptr;
    }

    binop_expr_t* result = match_binop(p);

    if (result == nullptr) {
        left->undo(p);
        delete left;
        return nullptr;
    }

    expr_t* right = match_expr(p);
    if (right == nullptr) {
        
        // Put back operator token
        p->put_back_token(result->tokens.back());
        result->tokens.pop_back();
        
        left->undo(p);
        delete left;
        delete result;
        return nullptr;
    }

    // If gotten this far, match was successful

    // Build syntax object 
    result->term = left;
    result->rest = right;

    return result;
}

// expr -> term
term_t* parser_t::match_expr_term(parser_t* p) {
    
    return match_term(p);
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

// term -> id ( params )
call_term_t* parser_t::match_term_call(parser_t* p) {

    lex::token* id_token;
    lex::token* open_paren_token;
    lex::token* closed_paren_token;

    id_token = p->get_token();
    open_paren_token = p->get_token();


    if (id_token->tag != lex::tag_t::ID || open_paren_token->tag != lex::tag_t::OPEN_PAREN) {
        p->put_back_token(open_paren_token);
        p->put_back_token(id_token);
        return nullptr;
    }

    params_t* params = match_params(p);

    closed_paren_token = p->get_token();

    if (closed_paren_token->tag != lex::tag_t::CLOSED_PAREN) {
        p->put_back_token(closed_paren_token);

        if (params != nullptr) {
            params->undo(p);
            delete params;
        }

        p->put_back_token(open_paren_token);
        p->put_back_token(id_token);
        return nullptr;
    }

    // If gotten this far, match was successful

    // Build syntax object
    call_term_t* result = new call_term_t();
    result->function_identifier = static_cast<lex::id_token*>(id_token)->lexeme;
    result->params = params;

    // Store tokens
    result->tokens.push_back(id_token);
    result->tokens.push_back(open_paren_token);
    result->tokens.push_back(closed_paren_token);

    return result;
}

// term -> ( expr )
expr_term_t* parser_t::match_term_expr(parser_t* p) {

    std::cout << "Matching expression term " << std::endl;
    lex::token* open_paren_token;
    lex::token* closed_paren_token;

    open_paren_token = p->get_token();

    if (open_paren_token->tag != lex::tag_t::OPEN_PAREN) {
        p->put_back_token(open_paren_token);
        return nullptr;
    }

    expr_t* expr = match_expr(p);

    if (expr == nullptr) {
        p->put_back_token(open_paren_token);
        return nullptr;
    }

    closed_paren_token = p->get_token();

    if (closed_paren_token->tag != lex::tag_t::CLOSED_PAREN) {
        p->put_back_token(closed_paren_token);
        expr->undo(p);
        p->put_back_token(open_paren_token);
        delete expr;
        return nullptr;
    }

    // If gotten this far, match was successful
    
    // Try rewriting associativity of expr
    expr = binop_expr_t::rewrite(expr);

    // Build syntax object
    expr_term_t* result = new expr_term_t();
    result->expr = expr;

    // Store token
    result->tokens.push_back(open_paren_token);
    result->tokens.push_back(closed_paren_token);
    return result;
}
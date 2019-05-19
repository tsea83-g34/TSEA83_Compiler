
#include "../include/parser.h"
#include "../include/parser_types.h"
#include "../include/helper_functions.h"

#include <iostream>

void syntax_error::throw_error(const std::string& message, const lex::token* node) {
    auto line = std::to_string(node->line_number);
    auto column = std::to_string(node->column_number);
    throw syntax_error("\033[0;31m--- Syntax Error\033[0m " + line + ":" + column + "\033[0;31m:\033[0m  " + message + "\n");
}

parser_t::parser_t(lex::lexer *l) : lexical_analyzer(l) {

    type_map = std::unordered_map<std::string, int>();
    type_map.insert({"int", 0});
    type_map.insert({"char", 1});
    type_map.insert({"long", 2});

    type_name_map = std::unordered_map<int, std::string>();
    type_name_map.insert({0, "int"});
    type_name_map.insert({1, "char"});
    type_name_map.insert({2, "long"});

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
    return match_program();
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

program_t* parser_t::match_program() {

    decls_t* d = match_decls();

    if (d == nullptr) {
        
        return nullptr;
    }
    
    program_t *program = new program_t();
    program->decls = d;
    return program;
}

decls_t* parser_t::match_decls() {

    decls_t* ds = nullptr;

    lex::token* test_token = get_token();
    lex::tag_t tag = test_token->tag;
    put_back_token(test_token);
    
    if (tag == lex::tag_t::eof) return nullptr;
    
    // Try matching decl, if it fails, check if next token is EOF, if it is, everything is in order
    // If it is not rethrow the syntax error
    try {
        ds = match_decls_1();
    } catch (syntax_error e) {

        if (peek()->tag != lex::tag_t::eof) {
            std::string id_str = (peek()->tag == lex::tag_t::ID) ? static_cast<const lex::id_token*>(peek())->lexeme : "";
            throw e;
        }
    }

    if (ds != nullptr) return ds;

    return match_decls_2();
}

decl_t* parser_t::match_decl() {
    decl_t* d;

    d = match_decl_var();
    if (d != nullptr) return d;
    
    d = match_decl_array();
    if (d != nullptr) return d;

    d = match_decl_func();
    if (d != nullptr) return d;

    syntax_error::throw_error("Could not match declaration. Unexpected " + lex::token_names[(int) peek()->tag] + " token ", peek());
}

func_decl_t* parser_t::match_decl_func() {

    func_decl_t* d;

    d = match_decl_func_1();
    if (d != nullptr) return d;

    d = match_decl_func_2();
    return d;
}

var_decl_t* parser_t::match_decl_var() {
    
    // This one is simple, since one of the productions of a variable declaration is the prefix of
    // the other. So both have been implemented in the same function. There is a separate function
    // that checks the common suffix, and the one called here checks for the rest of the other
    // production
    var_decl_t* result = match_decl_var_2();
    
    // If failed to match declaration, return
    if (result == nullptr) return nullptr;
    
    // Acquire semi colon
    lex::token* semi_colon_token = get_token();

    if (semi_colon_token->tag != lex::tag_t::SEMI_COLON) {

        put_back_token(semi_colon_token);
        result->undo(this);
        delete result;
        return nullptr;
    }

    result->tokens.push_back(semi_colon_token);
    return result;
}

array_decl_t* parser_t::match_decl_array() {

    array_decl_t* result = nullptr;
    
    result = match_decl_array_simple();
    if (result != nullptr) return result;

    result = match_decl_array_init_list();
    if (result != nullptr) return result;

    result = match_decl_array_str();
    return result;
}

// param_decls -> param params
//             |  e  
param_decls_t* parser_t::match_param_decls() {

    param_decl_t* first = match_param_decl();
    // If failed matching first, return nullptr
    if (first == nullptr) {
        return nullptr;
    }

    param_decls_t* result = new param_decls_t();
    result->first = first;

    // TODO add syntax error handling here

    // Try matching more decls
    result->rest = match_param_decls();
    
    return result;
}

// param_decl -> type id
//            |  type * id
param_decl_t* parser_t::match_param_decl() {

    lex::token* type_token;
    lex::token* star_token = nullptr;
    lex::token* id_token;

    type_token  = get_token();
    
    star_token = get_token();
    if (star_token->tag != lex::tag_t::STAR) {
        put_back_token(star_token);
        star_token = nullptr;
    }

    id_token    = get_token();

    if (!is_type(type_token) || id_token->tag != lex::tag_t::ID) {
        put_back_token(id_token);
        if (star_token) put_back_token(star_token);
        put_back_token(type_token);
        return nullptr;
    }

    // If gotten this far, match was succesful

    // Create syntax object
    param_decl_t* result = new param_decl_t();
    result->type        = get_type(static_cast<lex::id_token*>(type_token));
    result->id          = static_cast<lex::id_token*>(id_token)->lexeme;
    result->is_pointer  = star_token != nullptr;

    // Store token
    result->tokens.push_back(type_token);
    if (star_token) result->tokens.push_back(star_token);
    result->tokens.push_back(id_token);

    return result;
}

//  params      ->  expr params
//              |   e
params_t* parser_t::match_params() {

    expr_t* first;

    try {
        first = match_expr();
    } catch (syntax_error e) {
        return nullptr;
    }
    
    // Change associativity of expression if needed
    first = binop_expr_t::rewrite(first);
    

    params_t* result = new params_t();
    result->first = first;

    // Try matching more parameters
    result->rest = match_params();

    return result;
}

asm_params_t* parser_t::match_asm_params() {

    asm_param_t* first;

    first = match_asm_param();

    if (first == nullptr) return nullptr;

    asm_params_t* result = new asm_params_t();
    result->first = first;

    // Try matching more parameters
    result->rest = match_asm_params();

    return result;
}

asm_param_t* parser_t::match_asm_param() {

    asm_param_t* result;

    result = match_term_literal();
    if (result != nullptr) return result;

    result = match_term_identifier();
    return result;

}

stmts_t* parser_t::match_stmts() {
    
    stmts_t* stmts = nullptr;

    try {
        stmts = match_stmts_1();
    } catch (syntax_error e) {

        if (peek()->tag != lex::tag_t::CLOSED_BRACE) {
            throw e;
        }
    }
    if (stmts != nullptr) return stmts;

    return match_stmts_2();
}

stmt_t* parser_t::match_stmt() {
    
    stmt_t* stmt;

    stmt = match_stmt_assign();
    if (stmt != nullptr) return stmt;

    stmt = match_stmt_assign_deref();
    if (stmt != nullptr) return stmt;

    stmt = match_stmt_assign_indexed();
    if (stmt != nullptr) return stmt;

    stmt = match_stmt_decl();
    if (stmt != nullptr) return stmt;

    stmt = match_decl_array();
    if (stmt != nullptr) return stmt;

    stmt = match_stmt_block();
    if (stmt != nullptr) return stmt;

    stmt = match_stmt_return();
    if (stmt != nullptr) return stmt;

    stmt = match_stmt_expr();
    if (stmt != nullptr) return stmt;

    stmt = match_stmt_if();
    if (stmt != nullptr) return stmt;

    stmt = match_stmt_while();
    if (stmt != nullptr) return stmt;

    stmt = match_stmt_asm();
    if (stmt != nullptr) return stmt;

    syntax_error::throw_error("Could not match statement. Unexpected " + lex::token_names[(int) peek()->tag] + " token ", peek());
}

expr_t* parser_t::match_expr() {
    
    expr_t* expr;

    expr = match_expr_binop();
    if (expr != nullptr) return expr;

    expr = match_expr_term();
    if (expr != nullptr) return expr;

    expr = match_expr_negated();
    if (expr != nullptr) return expr;

    expr = match_expr_not();
    if (expr != nullptr) return expr;

    syntax_error::throw_error("Could not match expression. Unexpected " + lex::token_names[(int) peek()->tag] + " token ", peek());
}

binop_expr_t* parser_t::match_binop() {
   
    lex::token* op_token = get_token();
    binop_expr_t* result = nullptr; 

    switch (op_token->tag) {
        case lex::tag_t::PLUS:
            result = new add_binop_t();
            break;
        case lex::tag_t::MINUS:
            result = new sub_binop_t();
            break;
        case lex::tag_t::AND:
            result = new and_binop_t();
            break;
        case lex::tag_t::OR:
            result = new or_binop_t();
            break;
        case lex::tag_t::STAR:
            result = new mult_binop_t();
            break;
        case lex::tag_t::EQUALS:
            result = new eq_binop_t();
            break;
        case lex::tag_t::NOT_EQUALS:
            result = new neq_binop_t();
            break;
        case lex::tag_t::LESS:
            result = new less_binop_t();
            break;
        case lex::tag_t::GREATER:
            result = new greater_binop_t();
            break;
        case lex::tag_t::LESS_OR_EQUAL:
            result = new less_eq_binop_t();
            break;
        case lex::tag_t::GREATER_OR_EQUAL:
            result = new greater_eq_binop_t();
            break;
        default:
            put_back_token(op_token);
            return nullptr;
    }

    // If gotten this far, match was successful
    
    // Store token
    result->tokens.push_back(op_token);

    return result;
}

term_t* parser_t::match_term() {
    
    term_t* term;

    term = match_term_call();
    if (term != nullptr) return term;
    
    
    term = match_term_literal();
    if (term != nullptr) return term;

    term = match_term_addr_of();
    if (term != nullptr) return term;

    term = match_term_deref();
    if (term != nullptr) return term;
    
    // Cannot be below match_term_identifier since that is a substring of this production
    term = match_term_indexed();
    if (term != nullptr) return term;

    term = match_term_identifier();
    if (term != nullptr) return term;

    term = match_term_expr();
    return term;

}

// Production specific matching functions

// decls -> decl decls
decls_t* parser_t::match_decls_1() {

    decl_t* first = match_decl();

    // If could not find a declaration, free memory and return nullptr
    if (first == nullptr) {
        return nullptr;
    }
    decls_t* result = new decls_t;
    result->first = first;

    // Otherwise, look for more declarations
    decls_t* rest = (decls_t*) match_decls();
    result->rest = rest;

    return result;
}

// decls -> e
decls_t* parser_t::match_decls_2() {
    // Empty production should just return nullptr, 
    // since empty production in struct means nullptr
    // and empty production always matches
    return nullptr;
}

// var_decl -> type id
//          |  type * id
var_decl_t* parser_t::match_decl_var_1() {

    lex::token *type_token;
    lex::token *star_token = nullptr;
    lex::token *id_token;
    // Get type
    type_token = get_token();

    // If first token is not a word token or is not a type, put back token, delete d and return nullptr
    if (!is_type(type_token)) {
        put_back_token(type_token);
        return nullptr;
    }

    star_token = get_token();
    if (star_token->tag != lex::tag_t::STAR) {
        put_back_token(star_token);
        star_token = nullptr;
    }

    // Get identifier
    id_token = get_token();
    
    // If token is not an identifier, put back tokens in reverse order, delete d and return nullptr
    if (id_token->tag != lex::tag_t::ID) {
        put_back_token(id_token);
        put_back_token(type_token);
        return nullptr;
    }

    // If successful, build syntax type
    var_decl_t* d = new var_decl_t();
    
    d->type = get_type(static_cast<lex::id_token*>(type_token));
    d->id = static_cast<lex::id_token*>(id_token)->lexeme;
    d->is_pointer = (star_token != nullptr);

    d->tokens.push_back(type_token);
    d->tokens.push_back(id_token);

    return d;
}

// var_decl -> type id "=" expr
//          |  type "*" id "=" expr
var_decl_t* parser_t::match_decl_var_2() {

    // Use other function to try to find first part of the declaration
    var_decl_t* d  = match_decl_var_1();
    // If first part is not found, return nullptr
    if (d == nullptr) return nullptr;

    // Try to find assignment
    lex::token* equals = get_token();

    // If token is not an assignment operator put back token and return earlier declaration
    if (equals->tag != lex::tag_t::ASSIGNMENT) {
        put_back_token(equals);
        d->value = nullptr;
        return d;
    }

    // Else check for expression
    expr_t* value = match_expr();
    
    // If no expression is found, return nullptr
    // TODO: Throw exception instead and free memory
    if (value == nullptr) {
        
        value->undo(this);
        put_back_token(equals);
        d->undo(this);
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

init_list_t* parser_t::match_init_list() {

    expr_t* first;

    try {
        first = match_expr();
    } catch (syntax_error e) {
        return nullptr;
    }
    
    // Change associativity of expression if needed
    first = binop_expr_t::rewrite(first);
    

    init_list_t* result = new init_list_t();
    result->first = first;

    // Try matching more parameters
    result->rest = match_init_list();

    return result;    

}

// array_decl -> type id [ expr ] ;
simple_array_decl_t* parser_t::match_decl_array_simple() {

    lex::token* type_token;
    lex::token* identifier_token;
    lex::token* open_bracket_token;
    lex::token* closed_bracket_token;
    lex::token* semi_colon_token;

    type_token          = get_token();
    identifier_token    = get_token();
    open_bracket_token  = get_token();

    if (!is_type(type_token) || identifier_token->tag != lex::tag_t::ID || open_bracket_token->tag != lex::tag_t::OPEN_BRACKET) {
        put_back_token(open_bracket_token);
        put_back_token(identifier_token);
        put_back_token(type_token);
        return nullptr;
    }

    expr_t* size = nullptr;
    try {
        size = match_expr();
    } catch (syntax_error e) {
        put_back_token(open_bracket_token);
        put_back_token(identifier_token);
        put_back_token(type_token);
        return nullptr;
    }

    closed_bracket_token = get_token();
    semi_colon_token     = get_token();

    if (closed_bracket_token->tag != lex::tag_t::CLOSED_BRACKET || semi_colon_token->tag != lex::tag_t::SEMI_COLON) {
        put_back_token(semi_colon_token);
        put_back_token(closed_bracket_token);

        size->undo(this);
        delete size;

        put_back_token(open_bracket_token);
        put_back_token(identifier_token);
        put_back_token(type_token);
        return nullptr;
    }

    // If gotten this far, match was successful

    // Change associativity of size expressinon
    size = binop_expr_t::rewrite(size);

    // Build syntax object
    simple_array_decl_t* result = new simple_array_decl_t();
    
    result->type = get_type(static_cast<lex::id_token*>(type_token));
    result->identifier = static_cast<lex::id_token*>(identifier_token)->lexeme;
    result->size = size;

    // Store tokens
    result->tokens.push_back(type_token);
    result->tokens.push_back(identifier_token);
    result->tokens.push_back(open_bracket_token);
    result->tokens.push_back(closed_bracket_token);
    result->tokens.push_back(semi_colon_token);

    return result;
}

// array_decl -> type id [ ] = { init_list } ;
init_list_array_decl_t* parser_t::match_decl_array_init_list() {

    lex::token* type_token;
    lex::token* identifier_token;
    lex::token* open_bracket_token;
    lex::token* closed_bracket_token;
    lex::token* assignment_token;
    lex::token* open_brace_token;
    lex::token* closed_brace_token;
    lex::token* semi_colon_token;

    type_token              = get_token();
    identifier_token        = get_token();   
    open_bracket_token      = get_token();
    closed_bracket_token    = get_token();  
    assignment_token        = get_token();
    open_brace_token        = get_token();

    if (!is_type(type_token) ||
        identifier_token->tag != lex::tag_t::ID ||
        open_bracket_token->tag != lex::tag_t::OPEN_BRACKET ||
        closed_bracket_token->tag != lex::tag_t::CLOSED_BRACKET ||
        assignment_token->tag != lex::tag_t::ASSIGNMENT ||
        open_brace_token->tag != lex::tag_t::OPEN_BRACE) {

            put_back_token(open_brace_token);
            put_back_token(assignment_token);
            put_back_token(closed_bracket_token);
            put_back_token(open_bracket_token);
            put_back_token(identifier_token);
            put_back_token(type_token);
            return nullptr;
    }

    init_list_t* init_list = match_init_list();

    if (init_list == nullptr) {
        put_back_token(open_brace_token);
        put_back_token(assignment_token);
        put_back_token(closed_bracket_token);
        put_back_token(open_bracket_token);
        put_back_token(identifier_token);
        put_back_token(type_token);
        return nullptr;
    }

    closed_brace_token = get_token();
    semi_colon_token   = get_token();

    if (closed_brace_token->tag != lex::tag_t::CLOSED_BRACE || semi_colon_token->tag != lex::tag_t::SEMI_COLON) {
        put_back_token(semi_colon_token);
        put_back_token(closed_brace_token);

        init_list->undo(this);
        delete init_list;

        put_back_token(open_brace_token);
        put_back_token(assignment_token);
        put_back_token(closed_bracket_token);
        put_back_token(open_bracket_token);
        put_back_token(identifier_token);
        put_back_token(type_token);
        return nullptr;
    }

    // If gotten this far, match was successful

    // Build syntax object
    init_list_array_decl_t* result = new init_list_array_decl_t();

    result->type = get_type(static_cast<lex::id_token*>(type_token));
    result->identifier = static_cast<lex::id_token*>(identifier_token)->lexeme;
    result->init_list = init_list;

    // Store tokens
    result->tokens.push_back(type_token);
    result->tokens.push_back(identifier_token);
    result->tokens.push_back(open_bracket_token);
    result->tokens.push_back(closed_bracket_token);
    result->tokens.push_back(assignment_token);
    result->tokens.push_back(open_brace_token);
    result->tokens.push_back(closed_brace_token);
    result->tokens.push_back(semi_colon_token);

    return result;
}

// array_decl -> type id [ ] = str_lit ; 
str_array_decl_t* parser_t::match_decl_array_str() {

    lex::token* type_token              = get_token();
    lex::token* identifier_token        = get_token();
    lex::token* open_bracket_token      = get_token();
    lex::token* closed_bracket_token    = get_token();
    lex::token* assignment_token        = get_token();
    lex::token* string_literal_token    = get_token();
    lex::token* semi_colon_token        = get_token();


    if (!is_type(type_token) ||
        identifier_token->tag != lex::tag_t::ID ||
        open_bracket_token->tag != lex::tag_t::OPEN_BRACKET ||
        closed_bracket_token->tag != lex::tag_t::CLOSED_BRACKET ||
        assignment_token->tag != lex::tag_t::ASSIGNMENT ||
        string_literal_token->tag != lex::tag_t::STRING_LITERAL ||
        semi_colon_token->tag != lex::tag_t::SEMI_COLON) {

            put_back_token(semi_colon_token);
            put_back_token(string_literal_token);
            put_back_token(assignment_token);
            put_back_token(closed_bracket_token);
            put_back_token(open_bracket_token);
            put_back_token(identifier_token);
            put_back_token(type_token);
            return nullptr;
    }

    // If gotten this far, match was successful

    // Build syntax object
    str_array_decl_t* result = new str_array_decl_t();
    
    result->type = get_type(static_cast<lex::id_token*>(type_token));
    result->identifier = static_cast<lex::id_token*>(identifier_token)->lexeme;
    result->string_literal = static_cast<lex::str_literal_token*>(string_literal_token)->value;

    // Store tokens
    result->tokens.push_back(type_token);
    result->tokens.push_back(identifier_token);
    result->tokens.push_back(open_bracket_token);
    result->tokens.push_back(closed_bracket_token);
    result->tokens.push_back(assignment_token);
    result->tokens.push_back(string_literal_token);
    result->tokens.push_back(semi_colon_token);

    return result;
}

// func_decl -> type id ( param_decls ) ;
func_decl_t* parser_t::match_decl_func_1() {

    lex::token* type_token;
    lex::token* id_token;
    lex::token* open_paren_token;
    lex::token* closed_paren_token;
    lex::token* semi_token;

    // Acquire first token
    type_token = get_token();

    // If first token is not a type token, put back token, delete d and return nullptr
    if (!is_type(type_token)) {
        put_back_token(type_token);
        return nullptr;
    }

    // Acquire identifier token
    id_token = get_token();
    // If the acquired token is not an identifier, put back tokens, delete d and return nullptr
    if (id_token->tag != lex::tag_t::ID) {
        put_back_token(id_token);
        put_back_token(type_token);
        return nullptr;
    }

    // Acquire first parenthesis tokens
    open_paren_token    = get_token();

    if (open_paren_token->tag != lex::tag_t::OPEN_PAREN) {
        put_back_token(open_paren_token);
        put_back_token(id_token);
        put_back_token(type_token);
        return nullptr;
    }

    // Try matching parameter declarations, if none is found, it's not a problem
    param_decls_t* param_list = match_param_decls();


    closed_paren_token  = get_token();
    semi_token          = get_token();
    // If acquired tokens are not parenthesis
    if (closed_paren_token->tag != lex::tag_t::CLOSED_PAREN || semi_token->tag != lex::tag_t::SEMI_COLON) {
        
        put_back_token(semi_token);
        put_back_token(closed_paren_token);

        if (param_list != nullptr) {
            param_list->undo(this);
            delete param_list;
        }

        put_back_token(open_paren_token);
        put_back_token(id_token);
        put_back_token(type_token);
        return nullptr;
    }

    // If gotten so far, match is successful.
    // Build syntax object
    func_decl_t* result = new func_decl_t();
    result->type = get_type(static_cast<lex::id_token*>(type_token));
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
func_decl_t* parser_t::match_decl_func_2() {

    lex::token* type_token;
    lex::token* id_token;
    lex::token* open_paren_token;
    lex::token* closed_paren_token;

    // Acquire first token
    type_token = get_token();

    // If first token is not a type token, put back token, delete d and return nullptr
    if (!is_type(type_token)) {
        put_back_token(type_token);
        return nullptr;
    }

    // Acquire identifier token
    id_token = get_token();
    // If the acquired token is not an identifier, put back tokens, delete d and return nullptr
    if (id_token->tag != lex::tag_t::ID) {
        put_back_token(id_token);
        put_back_token(type_token);
        return nullptr;
    }

    // Acquire first parenthesis tokens
    open_paren_token = get_token();

    if (open_paren_token->tag != lex::tag_t::OPEN_PAREN) {
        put_back_token(open_paren_token);
        put_back_token(id_token);
        put_back_token(type_token);
        return nullptr;
    }

    // Try matching parameter declarations, if none is found, it's not a problem
    param_decls_t* param_list = match_param_decls();

    closed_paren_token  = get_token();
    // If acquired tokens are not parenthesis
    if (closed_paren_token->tag != lex::tag_t::CLOSED_PAREN) {
        put_back_token(closed_paren_token);

        // If there is a parameter list, undo it
        if (param_list != nullptr) {
            param_list->undo(this);
            delete param_list;
        }

        put_back_token(open_paren_token);
        put_back_token(id_token);
        put_back_token(type_token);
        return nullptr;
    }

    // Acquire block statement
    block_stmt_t* bs = match_stmt_block();
    
    // If unsuccessful in finding block statement, put back tokens and return nullptr
    if (bs == nullptr) {
        put_back_token(closed_paren_token);

        // If there is a parameter list, undo it
        if (param_list != nullptr) {
            param_list->undo(this);
            delete param_list;
        }

        put_back_token(open_paren_token);
        put_back_token(id_token);
        put_back_token(type_token);
        return nullptr;
    }

    // If gotten so far, match is successful
    // Build syntax object
    func_decl_t* result = new func_decl_t();
    result->type = get_type(static_cast<lex::id_token*>(type_token));
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
block_stmt_t* parser_t::match_stmt_block() {
    
    lex::token* open_brace;
    lex::token* closed_brace;

    // Acquire first token
    open_brace = get_token();

    // If first token is not an open brace, revert
    if (open_brace->tag != lex::tag_t::OPEN_BRACE) {
        put_back_token(open_brace);
        return nullptr;
    }
    

    stmts_t* inner = match_stmts();
    // Not matching statements is okay
    /*if (inner == nullptr) {
        put_back_token(open_brace);
        return nullptr;
    }*/

    

    // Get token for closed brace
    closed_brace = get_token();
    if (closed_brace->tag != lex::tag_t::CLOSED_BRACE) {
        
        
        put_back_token(closed_brace);
        inner->undo(this);
        put_back_token(open_brace);

        delete inner;
        return nullptr;
    }

    

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
//      |  if ( expr ) stmt else stmt 
if_stmt_t* parser_t::match_stmt_if() {

    

    lex::token* if_token;
    lex::token* open_paren_token;
    lex::token* closed_paren_token;
    lex::token* else_token;

    if_token = get_token();
    open_paren_token = get_token();
    
    // Mismatching tokens, revert
    if (if_token->tag != lex::tag_t::IF || open_paren_token->tag != lex::tag_t::OPEN_PAREN) {
        put_back_token(open_paren_token);
        put_back_token(if_token);
        return nullptr;
    }

    // Acquire conditional expression
    expr_t* cond;

    try {
        cond = match_expr();
    } catch (syntax_error e) {

        // If could not acquire conditional, revert
        put_back_token(open_paren_token);
        put_back_token(if_token);
        syntax_error::throw_error("Could not match condition expression for if statement. Unexpected " + lex::token_names[(int) peek()->tag] + " token ", peek());
    }

    closed_paren_token = get_token();

    if (closed_paren_token->tag != lex::tag_t::CLOSED_PAREN) {
        put_back_token(closed_paren_token);
        cond->undo(this);
        put_back_token(open_paren_token);
        put_back_token(if_token);

        delete cond;
        return nullptr;
    }

    stmt_t* stmt = match_stmt();

    if (stmt == nullptr) {
        put_back_token(closed_paren_token);
        cond->undo(this);
        put_back_token(open_paren_token);
        put_back_token(if_token);
        delete cond;
        return nullptr;
    }

    else_token = get_token();
    stmt_t* else_stmt = nullptr;

    // Try matching else statement
    if (else_token->tag == lex::tag_t::ELSE) {

        else_stmt = match_stmt();

    } else {
        put_back_token(else_token);
        else_token = nullptr;
    }

    // If we got this far, we have a successful match, delete tokens and return result
    
    // Change associativity of expression if needed
    cond = binop_expr_t::rewrite(cond);
    

    // Build syntax object
    if_stmt_t* result = new if_stmt_t();
    result->cond = cond;
    result->actions = stmt;
    result->else_actions = else_stmt;

    // Store tokens
    result->tokens.push_back(if_token);
    result->tokens.push_back(open_paren_token);
    result->tokens.push_back(closed_paren_token);
    if (else_token) result->tokens.push_back(else_token);

    
    return result;
}

// stmt -> while ( expr ) stmt
while_stmt_t* parser_t::match_stmt_while() {

    

    lex::token* while_token;
    lex::token* open_paren_token;
    lex::token* closed_paren_token;

    while_token = get_token();
    open_paren_token = get_token();
    
    // Mismatching tokens, revert
    if (while_token->tag != lex::tag_t::WHILE || open_paren_token->tag != lex::tag_t::OPEN_PAREN) {
        put_back_token(open_paren_token);
        put_back_token(while_token);
        return nullptr;
    }

    // Acquire conditional expression
    expr_t* cond;

    try {
        cond = match_expr();
    } catch (syntax_error e) {

        // If could not acquire conditional, revert
        put_back_token(open_paren_token);
        put_back_token(while_token);
        syntax_error::throw_error("Could not match condition expression for if statement. Unexpected " + lex::token_names[(int) peek()->tag] + " token ", peek());
    }

    closed_paren_token = get_token();

    if (closed_paren_token->tag != lex::tag_t::CLOSED_PAREN) {
        put_back_token(closed_paren_token);
        cond->undo(this);
        put_back_token(open_paren_token);
        put_back_token(while_token);

        delete cond;
        return nullptr;
    }

    stmt_t* stmt = match_stmt();

    if (stmt == nullptr) {
        put_back_token(closed_paren_token);
        cond->undo(this);
        put_back_token(open_paren_token);
        put_back_token(while_token);
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

    
    return result;
}

// stmt -> asm ( str_lit asm_params ) ; 
asm_stmt_t* parser_t::match_stmt_asm() {

    lex::token* asm_token;
    lex::token* open_paren_token;
    lex::token* string_literal_token;
    lex::token* closed_paren_token;
    lex::token* semi_colon_token;

    asm_token               = get_token();
    open_paren_token        = get_token();
    string_literal_token    = get_token();

    if (asm_token->tag != lex::tag_t::ASM || open_paren_token->tag != lex::tag_t::OPEN_PAREN || string_literal_token->tag != lex::tag_t::STRING_LITERAL) {
        put_back_token(string_literal_token);
        put_back_token(open_paren_token);
        put_back_token(asm_token);
        return nullptr;
    }

    asm_params_t* params = match_asm_params();

    if (params == nullptr) {
        put_back_token(string_literal_token);
        put_back_token(open_paren_token);
        put_back_token(asm_token);
        return nullptr;
    }

    closed_paren_token = get_token();
    semi_colon_token = get_token();

    if (closed_paren_token->tag != lex::tag_t::CLOSED_PAREN || semi_colon_token->tag != lex::tag_t::SEMI_COLON) {
        
        put_back_token(semi_colon_token);
        put_back_token(closed_paren_token);

        params->undo(this);
        delete params;

        put_back_token(string_literal_token);
        put_back_token(open_paren_token);
        put_back_token(asm_token);
        return nullptr;
    }

    // If gotten this far, match was successful

    // Build syntax object
    asm_stmt_t* result = new asm_stmt_t();
    result->literal = strip_quotations(static_cast<lex::str_literal_token*>(string_literal_token)->value); 
    result->params = params;

    // Store tokens
    result->tokens.push_back(asm_token);
    result->tokens.push_back(open_paren_token);
    result->tokens.push_back(string_literal_token);
    result->tokens.push_back(closed_paren_token);
    result->tokens.push_back(semi_colon_token);

    return result;
}


// stmt -> return expr ;
return_stmt_t* parser_t::match_stmt_return() {

    lex::token* return_token;
    lex::token* semi_colon_token;

    return_token = get_token();

    if (return_token->tag != lex::tag_t::RETURN) {
        put_back_token(return_token);
        return nullptr;
    }
    
    expr_t* return_value;

    // Try matching return value expression
    try {
        return_value = match_expr();
    } catch (syntax_error e) {
        put_back_token(return_token);
        syntax_error::throw_error("Could not match expression for return statement. Unexpected " + lex::token_names[(int) peek()->tag] + " token ", peek());
    }

    semi_colon_token = get_token();

    if (semi_colon_token->tag != lex::tag_t::SEMI_COLON) {
        
        put_back_token(semi_colon_token);
        return_value->undo(this);
        put_back_token(return_token);

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
expr_stmt_t* parser_t::match_stmt_expr() {

    lex::token* semi_colon_token;

    expr_t* e = nullptr;
    try {
        e = match_expr();
    } catch (syntax_error e) {

        // If could not match expression, return nullptr
        return nullptr;
    }

    semi_colon_token = get_token();

    if (semi_colon_token->tag != lex::tag_t::SEMI_COLON) {
        put_back_token(semi_colon_token);
        e->undo(this);
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
var_decl_t* parser_t::match_stmt_decl() {
    
    var_decl_t* result = match_decl_var();

    // If could not match a variable declaration, return nullptr
    if (result == nullptr) {
        return nullptr;
    }
    return result;
}

// stmt -> id "=" expr ;
assignment_stmt_t* parser_t::match_stmt_assign() {

    lex::token* id_token;
    lex::token* assign_token;
    lex::token* semi_colon_token;

    id_token     = get_token();
    assign_token = get_token();

    // If first two tokens don't match, revert
    if (id_token->tag != lex::tag_t::ID || assign_token->tag != lex::tag_t::ASSIGNMENT) {
        put_back_token(assign_token);
        put_back_token(id_token);
        return nullptr;
    }

    // Acquire identifier from token
    expr_t* rvalue;

    try {
        rvalue = match_expr();
    } catch (syntax_error e) {
        // If could not match expression, revert
        
        put_back_token(assign_token);
        put_back_token(id_token);
        return nullptr;
    }

    semi_colon_token = get_token();

    if (semi_colon_token->tag != lex::tag_t::SEMI_COLON) {
        put_back_token(semi_colon_token);
        rvalue->undo(this);
        put_back_token(assign_token);
        put_back_token(id_token);
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

// stmt -> "*" id "=" expr ;
deref_assignment_stmt_t* parser_t::match_stmt_assign_deref() {

    lex::token* star_token;
    lex::token* id_token;
    lex::token* assign_token;
    lex::token* semi_colon_token;

    star_token = get_token();
    id_token   = get_token();
    assign_token = get_token();


    if (star_token->tag != lex::tag_t::STAR || id_token->tag != lex::tag_t::ID || assign_token->tag != lex::tag_t::ASSIGNMENT) {
        put_back_token(assign_token);
        put_back_token(id_token);
        put_back_token(star_token);
        return nullptr;
    }

    // Acquire identifier from token
    expr_t* rvalue;

    try {
        rvalue = match_expr();
    } catch (syntax_error e) {
        // If could not match expression, revert
        put_back_token(assign_token);
        put_back_token(id_token);
        put_back_token(star_token);
        return nullptr;
    }

    semi_colon_token = get_token();

    if (semi_colon_token->tag != lex::tag_t::SEMI_COLON) {
        put_back_token(semi_colon_token);
        rvalue->undo(this);
        put_back_token(assign_token);
        put_back_token(id_token);
        put_back_token(star_token);
        delete rvalue;
        return nullptr;
    }
    
    // If gotten this far, match was successful


    // Change associativity of expression if needed
    rvalue = binop_expr_t::rewrite(rvalue);
    

    // Build syntax object
    deref_assignment_stmt_t* result = new deref_assignment_stmt_t();
    result->identifier = static_cast<lex::id_token*>(id_token)->lexeme;
    result->rvalue = rvalue;

    // Store tokens
    result->tokens.push_back(star_token);
    result->tokens.push_back(id_token);
    result->tokens.push_back(assign_token);
    result->tokens.push_back(semi_colon_token);
    
    return result;
}

// stmt -> id [ expr ] "=" expr ;
indexed_assignment_stmt_t* parser_t::match_stmt_assign_indexed() {

    lex::token* id_token;
    lex::token* open_bracket_token;
    lex::token* closed_bracket_token;
    lex::token* assign_token;
    lex::token* semi_colon_token;

    id_token = get_token();
    open_bracket_token = get_token();


    if (id_token->tag != lex::tag_t::ID || open_bracket_token->tag != lex::tag_t::OPEN_BRACKET) {
        put_back_token(open_bracket_token);
        put_back_token(id_token);
        return nullptr;
    }

    // Acquire index expr from token
    expr_t* index;

    try {
        index = match_expr();
    } catch (syntax_error e) {
        // If could not match expression, revert
        put_back_token(open_bracket_token);
        put_back_token(id_token);
        return nullptr;
    }

    closed_bracket_token = get_token();
    assign_token = get_token();

    if (closed_bracket_token->tag != lex::tag_t::CLOSED_BRACKET || assign_token->tag != lex::tag_t::ASSIGNMENT) {
        
        put_back_token(assign_token);
        put_back_token(closed_bracket_token);
        
        index->undo(this);
        delete index;
        
        put_back_token(open_bracket_token);
        put_back_token(id_token);
        return nullptr;
    }

    expr_t* rvalue;

    try {
        rvalue = match_expr();
    } catch (syntax_error e) {
        // If could not match expression, revert
        put_back_token(assign_token);
        put_back_token(closed_bracket_token);
        
        index->undo(this);
        delete index;
        
        put_back_token(open_bracket_token);
        put_back_token(id_token);
        return nullptr;
    }

    semi_colon_token = get_token();

    if (semi_colon_token->tag != lex::tag_t::SEMI_COLON) {
        
        put_back_token(semi_colon_token);
        rvalue->undo(this);
        put_back_token(assign_token);
        put_back_token(closed_bracket_token);
        index->undo(this);
        put_back_token(open_bracket_token);
        put_back_token(id_token);
        
        delete index;
        delete rvalue;
        return nullptr;
    }
    
    // If gotten this far, match was successful


    // Change associativity of expression if needed
    rvalue = binop_expr_t::rewrite(rvalue);
    index = binop_expr_t::rewrite(index);

    // Build syntax object
    indexed_assignment_stmt_t* result = new indexed_assignment_stmt_t();
    result->identifier = static_cast<lex::id_token*>(id_token)->lexeme;
    result->rvalue = rvalue;
    result->index = index;

    // Store tokens
    result->tokens.push_back(id_token);
    result->tokens.push_back(open_bracket_token);
    result->tokens.push_back(closed_bracket_token);
    result->tokens.push_back(assign_token);
    result->tokens.push_back(semi_colon_token);
    return result;
}

// stmts -> stmt stmts
stmts_t* parser_t::match_stmts_1() {

    stmt_t* stmt = match_stmt();

    // If matching a single statement fails, return nullptr
    // A list of statements following this production must include at least one
    if (stmt == nullptr) {
        return nullptr;
    }

    stmts_t* result = new stmts_t();
    result->first = stmt;

    // Try matching more statements
    result->rest = match_stmts();
    
    return result;
}

// Maybe delete this function?
// stmts -> e
stmts_t* parser_t::match_stmts_2() {
    return nullptr;
}

// expr -> "-" term
neg_expr_t* parser_t::match_expr_negated() {
    
    lex::token* neg_token  = get_token();

    // If first token is not a minus operator, or the second token is not a literal nor an identifier, revert
    if (neg_token->tag != lex::tag_t::MINUS ) {
        put_back_token(neg_token);
        return nullptr;
    }

    term_t* value = match_term();
    
    if (value == nullptr) {
        put_back_token(neg_token);
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

// expr -> "!" term
not_expr_t* parser_t::match_expr_not() {
    
    lex::token* not_token  = get_token();

    // If first token is not a minus operator, or the second token is not a literal nor an identifier, revert
    if (not_token->tag != lex::tag_t::NOT) {
        put_back_token(not_token);
        return nullptr;
    }

    term_t* value = match_term();
    
    if (value == nullptr) {
        put_back_token(not_token);
        return nullptr;
    }

    // If gotten this far, match is successful

    // Create syntax object
    not_expr_t* result = new not_expr_t();
    result->value = value;

    // Store token
    result->tokens.push_back(not_token);
    
    return result;
}

binop_expr_t* parser_t::match_expr_binop() {

    term_t* left = match_term();

    if (left == nullptr) {
        return nullptr;
    }

    binop_expr_t* result = match_binop();

    if (result == nullptr) {
        left->undo(this);
        delete left;
        return nullptr;
    }

    expr_t* right = match_expr();
    if (right == nullptr) {
        
        // Put back operator token
        put_back_token(result->tokens.back());
        result->tokens.pop_back();
        
        left->undo(this);
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
term_t* parser_t::match_expr_term() {
    
    return match_term();
}

// term -> id
id_term_t* parser_t::match_term_identifier() {
    
    lex::token* id_token = get_token();

    if (id_token->tag != lex::tag_t::ID) {
        put_back_token(id_token);
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
lit_term_t* parser_t::match_term_literal() {
    
    lex::token* literal_token = get_token();

    if (literal_token->tag != lex::tag_t::INT_LITERAL) {
        put_back_token(literal_token);
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
call_term_t* parser_t::match_term_call() {

    lex::token* id_token;
    lex::token* open_paren_token;
    lex::token* closed_paren_token;

    id_token = get_token();
    open_paren_token = get_token();


    if (id_token->tag != lex::tag_t::ID || open_paren_token->tag != lex::tag_t::OPEN_PAREN) {
        put_back_token(open_paren_token);
        put_back_token(id_token);
        return nullptr;
    }

    params_t* params = match_params();

    closed_paren_token = get_token();

    if (closed_paren_token->tag != lex::tag_t::CLOSED_PAREN) {
        put_back_token(closed_paren_token);

        if (params != nullptr) {
            params->undo(this);
            delete params;
        }

        put_back_token(open_paren_token);
        put_back_token(id_token);
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

// term -> & id
addr_of_term_t* parser_t::match_term_addr_of() {
    
    lex::token* ampersand_token = get_token();
    lex::token* identifier_token = get_token();

    if (ampersand_token->tag != lex::tag_t::AND || identifier_token->tag != lex::tag_t::ID) {
        put_back_token(identifier_token);
        put_back_token(ampersand_token);
        return nullptr;
    }

    // If gotten this far, match was successful
    
    // Build syntax object
    addr_of_term_t* result = new addr_of_term_t();
    result->identifier = static_cast<lex::id_token*>(identifier_token)->lexeme;
    
    // Store token
    result->tokens.push_back(ampersand_token);
    result->tokens.push_back(identifier_token);

    return result;
}

// term -> * id
deref_term_t* parser_t::match_term_deref() {
    
    lex::token* star_token = get_token();
    lex::token* identifier_token = get_token();

    if (star_token->tag != lex::tag_t::STAR || identifier_token->tag != lex::tag_t::ID) {
        put_back_token(identifier_token);
        put_back_token(star_token);
        return nullptr;
    }

    // If gotten this far, match was successful
    
    // Build syntax object
    deref_term_t* result = new deref_term_t();
    result->identifier = static_cast<lex::id_token*>(identifier_token)->lexeme;
    
    // Store token
    result->tokens.push_back(star_token);
    result->tokens.push_back(identifier_token);

    return result;
}

// term -> id [ expr ]
indexed_term_t* parser_t::match_term_indexed() {

    lex::token* identifier_token;
    lex::token* open_bracket_token;
    lex::token* closed_bracket_token;

    identifier_token = get_token();
    open_bracket_token = get_token();

    if (identifier_token->tag != lex::tag_t::ID || open_bracket_token->tag != lex::tag_t::OPEN_BRACKET) {
        put_back_token(open_bracket_token);
        put_back_token(identifier_token);
        return nullptr;
    }

    expr_t* index = match_expr();

    if (index == nullptr) {
        put_back_token(open_bracket_token);
        put_back_token(identifier_token);
        return nullptr;
    }

    closed_bracket_token = get_token();
    
    if (closed_bracket_token->tag != lex::tag_t::CLOSED_BRACKET) {
        put_back_token(closed_bracket_token);

        index->undo(this);
        delete index;

        put_back_token(open_bracket_token);
        put_back_token(identifier_token);
        return nullptr;
    }

    // If gotten this far, match was successful

    // Try rewriting associativity of expr
    index = binop_expr_t::rewrite(index);

    // Build syntax object
    indexed_term_t* result = new indexed_term_t();

    result->identifier = static_cast<lex::id_token*>(identifier_token)->lexeme;
    result->index = index;

    // Store token
    result->tokens.push_back(identifier_token);
    result->tokens.push_back(open_bracket_token);
    result->tokens.push_back(closed_bracket_token);

    return result;
}


// term -> ( expr )
expr_term_t* parser_t::match_term_expr() {

    lex::token* open_paren_token;
    lex::token* closed_paren_token;

    open_paren_token = get_token();

    if (open_paren_token->tag != lex::tag_t::OPEN_PAREN) {
        put_back_token(open_paren_token);
        return nullptr;
    }

    expr_t* expr = match_expr();

    if (expr == nullptr) {
        put_back_token(open_paren_token);
        return nullptr;
    }

    closed_paren_token = get_token();

    if (closed_paren_token->tag != lex::tag_t::CLOSED_PAREN) {
        put_back_token(closed_paren_token);
        expr->undo(this);
        put_back_token(open_paren_token);
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
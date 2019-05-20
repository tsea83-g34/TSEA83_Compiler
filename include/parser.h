
#ifndef COM_PARSER_H
#define COM_PARSER_H

#include <unordered_map>
#include <vector>
#include <functional>
#include <deque>

#include "tokens.h"
#include "lexer.h"
#include "interfaces.h"

/* Predefine syntax tree structs */
struct program_t;

struct decl_t;
struct decls_t;
struct var_decl_t;
struct func_decl_t;

struct array_decl_t;

struct simple_array_decl_t;
struct init_list_array_decl_t;
struct str_array_decl_t;

struct init_list_t;

struct param_decls_t;
struct param_decl_t;
struct params_t;

struct stmt_t;
struct stmts_t;
struct block_stmt_t;
struct if_stmt_t;
struct while_stmt_t;
struct asm_stmt_t;
struct break_stmt_t;
struct continue_stmt_t;
struct assignment_stmt_t;
struct deref_assignment_stmt_t;
struct indexed_assignment_stmt_t;
struct return_stmt_t;
struct expr_stmt_t;

struct expr_t;
struct neg_expr_t;
struct not_expr_t;;
struct term_expr_t;

struct term_t;
struct lit_term_t;
struct id_term_t;
struct call_term_t;
struct expr_term_t;
struct deref_term_t;
struct addr_of_term_t;
struct indexed_term_t;

struct asm_params_t;
struct asm_param_t;
// New binary operation system

struct binop_expr_t;

struct add_binop_t;
struct sub_binop_t;
struct and_binop_t;
struct or_binop_t;
struct mult_binop_t;

struct eq_binop_t;
struct neq_binop_t;
struct less_biop_t;
struct greater_binop_t;

struct less_eq_binop_t;
struct greater_eq_binop_t;

/* ----------------------------- */


// Build syntax tree recursively, create function that matches each production. Matching function
// will search for the component terminals, and once a production is identified, call the semantic
// function of that production

class syntax_error : public std::exception {
private:
    std::string msg;
public:
    syntax_error(std::string _msg) : msg(_msg) {}
    ~syntax_error() {}

    const char* what() const noexcept { return msg.c_str(); }

    static void throw_error(const std::string& message, const lex::token* node);
};

class parser_t {
private:
    
    // The lexical analyzer used by the parser
    lex::lexer *lexical_analyzer;

    // Maps reserved types to their byte size
    // TODO: Implement proper type handling for composite types
    std::unordered_map<std::string, int> type_map;
    std::unordered_map<int, std::string> type_name_map;

    // Tokens will be loaded into this double ended queue to be processed
    std::deque<lex::token*> token_queue;
    size_t current_pos;

    inline lex::token* get_token();

    inline bool is_type(const std::string& s);
    inline bool is_type(const lex::token* t);

    inline int get_type(const std::string& s);
    inline int get_type(const lex::id_token* t);
    
    // Construct specific matching functions
    program_t* match_program();
    decls_t* match_decls();
    decl_t* match_decl();

    var_decl_t* match_decl_var();
    array_decl_t* match_decl_array();
    func_decl_t* match_decl_func();

    param_decls_t* match_param_decls();
    param_decl_t* match_param_decl();
    params_t* match_params();

    asm_params_t* match_asm_params();
    asm_param_t* match_asm_param();
    
    stmt_t* match_stmt();
    stmts_t* match_stmts();

    expr_t* match_expr();
    term_t* match_term();

    // Production specific matching functions
    decls_t* match_decls_1();
    decls_t* match_decls_2();

    var_decl_t* match_decl_var_1();
    var_decl_t* match_decl_var_2();

    init_list_t* match_init_list();

    simple_array_decl_t* match_decl_array_simple();
    init_list_array_decl_t* match_decl_array_init_list();
    str_array_decl_t* match_decl_array_str();

    func_decl_t* match_decl_func_1();
    func_decl_t* match_decl_func_2();

    block_stmt_t* match_stmt_block();
    if_stmt_t* match_stmt_if();
    while_stmt_t* match_stmt_while();
    asm_stmt_t* match_stmt_asm();
    var_decl_t* match_stmt_decl();
    assignment_stmt_t* match_stmt_assign();
    deref_assignment_stmt_t* match_stmt_assign_deref();
    indexed_assignment_stmt_t* match_stmt_assign_indexed();
    return_stmt_t* match_stmt_return();
    expr_stmt_t* match_stmt_expr();

    stmts_t* match_stmts_1();
    stmts_t* match_stmts_2();

    binop_expr_t* match_expr_binop();
    binop_expr_t* match_binop();

    neg_expr_t* match_expr_negated();
    not_expr_t* match_expr_not();
    term_t* match_expr_term();

    id_term_t* match_term_identifier();
    lit_term_t* match_term_literal();
    call_term_t* match_term_call();
    expr_term_t* match_term_expr();
    addr_of_term_t* match_term_addr_of();
    deref_term_t* match_term_deref();
    indexed_term_t* match_term_indexed();

public:
    parser_t(lex::lexer *l);
    ~parser_t();

    program_t* parse_token_stream();
    std::string get_type_name(int type);
    void put_back_token(lex::token* t);
    const lex::token* peek();
};

#endif
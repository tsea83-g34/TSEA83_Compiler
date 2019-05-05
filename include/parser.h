
#ifndef COM_PARSER_H
#define COM_PARSER_H

#include <unordered_map>
#include <vector>
#include <functional>
#include <deque>

#include "tokens.h"
#include "lexer.h"

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
struct while_stmt_t;
struct assignment_stmt_t;
struct return_stmt_t;
struct expr_stmt_t;

struct expr_t;
struct neg_expr_t;
struct not_expr_t;
struct term_expr_t;
struct binop_expr_t;

struct term_t;
struct lit_term_t;
struct id_term_t;
struct call_term_t;
struct expr_term_t;

struct binop_expr_t;

struct add_binop_t;
struct sub_binop_t;
struct and_binop_t;
struct or_binop_t;

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
    static program_t* match_program(parser_t* p);
    static decls_t* match_decls(parser_t* p);
    static decl_t* match_decl(parser_t* p);

    static var_decl_t* match_decl_var(parser_t* p);
    static func_decl_t* match_decl_func(parser_t* p);

    static param_decls_t* match_param_decls(parser_t* p);
    static param_decl_t* match_param_decl(parser_t* p);
    static params_t* match_params(parser_t* p);
    
    static stmt_t* match_stmt(parser_t* p);
    static stmts_t* match_stmts(parser_t* p);

    static expr_t* match_expr(parser_t* p);
    static term_t* match_term(parser_t* p);

    // Production specific matching functions
    static decls_t* match_decls_1(parser_t* p);
    static decls_t* match_decls_2(parser_t* p);

    static var_decl_t* match_decl_var_1(parser_t* p);
    static var_decl_t* match_decl_var_2(parser_t* p);

    static func_decl_t* match_decl_func_1(parser_t* p);
    static func_decl_t* match_decl_func_2(parser_t* p);

    static block_stmt_t* match_stmt_block(parser_t* p);
    static if_stmt_t* match_stmt_if(parser_t* p);
    static while_stmt_t* match_stmt_while(parser_t* p);
    static var_decl_t* match_stmt_decl(parser_t* p);
    static assignment_stmt_t* match_stmt_assign(parser_t* p);
    static return_stmt_t* match_stmt_return(parser_t* p);
    static expr_stmt_t* match_stmt_expr(parser_t* p);

    static stmts_t* match_stmts_1(parser_t* p);
    static stmts_t* match_stmts_2(parser_t* p);

    static binop_expr_t* match_expr_binop(parser_t* p);
    static binop_expr_t* match_binop(parser_t* p);

    static neg_expr_t* match_expr_negated(parser_t* p);
    static not_expr_t* match_expr_not(parser_t* p);
    static term_t* match_expr_term(parser_t* p);

    static id_term_t* match_term_identifier(parser_t* p);
    static lit_term_t* match_term_literal(parser_t* p);
    static call_term_t* match_term_call(parser_t* p);
    static expr_term_t* match_term_expr(parser_t* p);

public:
    parser_t(lex::lexer *l);
    ~parser_t();

    program_t* parse_token_stream();
    std::string get_type_name(int type);
    void put_back_token(lex::token* t);
    const lex::token* peek();
};

#endif
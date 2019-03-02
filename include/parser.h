
#ifndef COM_PARSER_H
#define COM_PARSER_H

#include <unordered_map>
#include <vector>
#include <functional>
#include <deque>

#include "parser_types.h"
#include "tokens.h"
#include "lexer.h"


// Build syntax tree recursively, create function that matches each production. Matching function
// will search for the component terminals, and once a production is identified, call the semantic
// function of that production

// Tags for productions, mostly inherited from tag_t
enum class prod_tags_t : int {

        /* Terminal types, inherited from tag_t */

        // Keywords
        IF = (int) lex::tag_t::IF,

        // Other
        ID = (int) lex::tag_t::ID,
        INT_LITERAL = (int) lex::tag_t::INT_LITERAL,
        STRING_LITERAL = (int) lex::tag_t::STRING_LITERAL,

        // Operators
        PLUS = (int) lex::tag_t::PLUS,
        MINUS = (int) lex::tag_t::MINUS,
        EQUALS = (int) lex::tag_t::EQUALS,
        NOT_EQUALS = (int) lex::tag_t::NOT_EQUALS,
        ASSIGNMENT = (int) lex::tag_t::ASSIGNMENT,
        
        // Special characters
        OPEN_PAREN = (int) lex::tag_t::OPEN_PAREN,
        CLOSED_PAREN = (int) lex::tag_t::CLOSED_PAREN,
        OPEN_BRACE = (int) lex::tag_t::OPEN_BRACE,
        CLOSED_BRACE = (int) lex::tag_t::CLOSED_BRACE,
        SEMI_COLON = (int) lex::tag_t::SEMI_COLON,
        
        // Probably remove these two
        eof = (int) lex::tag_t::eof,
        UNKNOWN = (int) lex::tag_t::UNKNOWN,

        /* Non-terminals newly added */

        PROGRAM,
        DECLS,
        DECL,
        VAR_DECL,
        FUNC_DECL,
        STMT,
        STMTS,
        BLOCK_STMT,
        EXPR,
        ARITHOP,
        RELOP,
        TERM,
        // Empty string
        EMPTY
}; 

// Hash function for enum classes
struct enum_class_hash_t {
    template <typename T>
    std::size_t operator()(T t) const {
        return static_cast<std::size_t>(t);
    }
};

// Struct for productions
struct prod_t {
    std::vector<prod_tags_t> rule;
    //std::function<ast_node_t*()> semantics;
};

class parser_t {
private:
    // Map of all productions, might be redundant
    std::unordered_map<prod_tags_t, std::vector<prod_t>, enum_class_hash_t> productions;
    
    // The lexical analyzer used by the aprser
    lex::lexer *lexical_analyzer;

    // Maps reserved types to their byte size
    // TODO: Implement proper type handling for composite types
    std::unordered_map<std::string, int> type_map;

    // Tokens will be loaded into this double ended queue to be processed
    std::deque<lex::token*> token_queue;
    size_t current_pos;

    void init_productions();

    lex::token* get_token();

    bool is_type(const std::string& s);
    
    // Construct specific matching functions
    static program_t* match_program(parser_t* p);
    static decls_t* match_decls(parser_t* p);
    static decl_t* match_decl(parser_t* p);
    static var_decl_t* match_decl_var(parser_t* p);
    static func_decl_t* match_decl_func(parser_t* p);
    static stmt_t* match_stmt(parser_t* p);
    static stmts_t* match_stmts(parser_t* p);
    static expr_t* match_expr(parser_t* p);
    static arithop_t* match_arithop(parser_t* p);
    static relop_t* match_relop(parser_t* p);
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
    static var_decl_t* match_stmt_decl(parser_t* p);
    static assignment_stmt_t* match_stmt_assign(parser_t* p);

    static stmts_t* match_stmts_1(parser_t* p);
    static stmts_t* match_stmts_2(parser_t* p);

    static arith_expr_t* match_expr_arithop(parser_t* p);
    static neg_expr_t* match_expr_negated(parser_t* p);
    static rel_expr_t* match_expr_relop(parser_t* p);
    static term_expr_t* match_expr_term(parser_t* p);

    static arithop_plus_t* match_arithop_plus(parser_t* p);
    static arithop_minus_t* match_arithop_minus(parser_t* p);

    static relop_equals_t* match_relop_equals(parser_t* p);
    static relop_not_equals_t* match_relop_not_equals(parser_t* p);

    static id_term_t* match_term_identifier(parser_t* p);
    static lit_term_t* match_term_literal(parser_t* p);
public:
    parser_t(lex::lexer *l);
    program_t* parse_token_stream();
};

#endif
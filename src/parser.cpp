
#include "../include/parser.h"



parser_t::parser_t() {
    init_productions();
}

void parser_t::init_productions() {

    productions = std::unordered_map<prod_tags_t, std::vector<prod_t>, enum_class_hash_t>();

    /* program  ->  decls  */
    std::vector<prod_t> program_productions = {
        (prod_t){{prod_tags_t::DECLS}, std::bind(&parser_t::match_program, this)}
    };
    productions.insert({prod_tags_t::PROGRAM, program_productions}); // PROGRAM::1

    // decls    ->  decl decls
    //          |   e       
    std::vector<prod_t> decls_productions = {
        (prod_t){{prod_tags_t::DECL, prod_tags_t::DECLS}, std::bind(&parser_t::match_decls_1, this)}, // DECLS::1
        (prod_t){{prod_tags_t::EMPTY}, std::bind(&parser_t::match_decls_2, this)}    // DECLS::2
    };
    productions.insert({prod_tags_t::DECLS, decls_productions});
    
    //  decl    ->  func_decl
    //          |   var_decl
    std::vector<prod_t> decl_productions = {
        (prod_t){{prod_tags_t::FUNC_DECL}, std::bind(&parser_t::match_decl_1, this)},
        (prod_t){{prod_tags_t::VAR_DECL}, std::bind(&parser_t::match_decl_2, this)}
    };
    productions.insert({prod_tags_t::DECL, decl_productions});

    //  var_decl    ->  type id
    //              |   type id "=" expr
    // TODO: Add type token!

    //  func_decl   ->  type id ( ) ;
    //              |   type id ( ) block_stmt
    // TODO: -||-

    //  stmt        ->  block_stmt
    //              |   if ( expr ) stmt
    //              |   var_decl ; 
    //              |   id "=" expr ;
    std::vector<prod_t> stmt_productions = {
        (prod_t){
            {prod_tags_t::BLOCK_STMT}, 
            std::bind(&parser_t::match_stmt_block, this)},
        
        (prod_t){
            {prod_tags_t::IF, prod_tags_t::OPEN_PAREN, prod_tags_t::EXPR, prod_tags_t::CLOSED_PAREN, prod_tags_t::STMT},
            std::bind(&parser_t::match_stmt_if, this)
    }};
    productions.insert({prod_tags_t::STMT, stmt_productions});

    //  stmts       ->  stmt stmts
    //              |   e
    std::vector<prod_t> stmts_productions = {
        (prod_t){{prod_tags_t::STMT, prod_tags_t::STMTS}, std::bind(&parser_t::match_stmts_1, this)},
        (prod_t){{prod_tags_t::EMPTY}, std::bind(&parser_t::match_stmts_2, this)}
    };
    productions.insert({prod_tags_t::STMTS, stmts_productions});

    //  block_stmt  ->  { stmts }
    std::vector<prod_t> block_stmt_productions = {
        (prod_t){{prod_tags_t::BLOCK_STMT}, std::bind(&parser_t::match_stmt_block, this)}
    };
    productions.insert({prod_tags_t::BLOCK_STMT, block_stmt_productions});

    //  expr        ->  term arithop expr
    //              |   "-" term
    //              |   term relop expr
    //              |   term
    std::vector<prod_t> expr_productions = {
        (prod_t){{prod_tags_t::TERM, prod_tags_t::ARITHOP, prod_tags_t::EXPR}, std::bind(&parser_t::match_expr_arithop, this)},
        (prod_t){{prod_tags_t::MINUS, prod_tags_t::TERM}, std::bind(&parser_t::match_expr_negated, this)},
        (prod_t){{prod_tags_t::TERM, prod_tags_t::RELOP, prod_tags_t::EXPR}, std::bind(&parser_t::match_expr_relop, this)},
        (prod_t){{prod_tags_t::TERM}, std::bind(&parser_t::match_expr_term, this)}
    };
    productions.insert({prod_tags_t::EXPR, expr_productions});

    //  arithop     ->  "+"
    //              |   "-"
    std::vector<prod_t> arithop_productions = {
        (prod_t){{prod_tags_t::PLUS}, std::bind(&parser_t::match_arithop_plus, this)},
        (prod_t){{prod_tags_t::MINUS}, std::bind(&parser_t::match_arithop_minus, this)}  
    };
    productions.insert({prod_tags_t::ARITHOP, arithop_productions});

    //  relop       ->  "=="
    //              |   "!="
    std::vector<prod_t> relop_productions = {
        (prod_t){{prod_tags_t::EQUALS}, std::bind(&parser_t::match_relop_equals, this)},
        (prod_t){{prod_tags_t::NOT_EQUALS}, std::bind(&parser_t::match_relop_not_equals, this)}
    };
    productions.insert({prod_tags_t::RELOP, relop_productions});

    //term        ->  id
    //            |   literal
    std::vector<prod_t> term_productions = {
        (prod_t){{prod_tags_t::ID}, std::bind(&parser_t::match_term_identifier, this)},
        (prod_t){{prod_tags_t::INT_LITERAL}, std::bind(&parser_t::match_term_literal, this)}
    };
    productions.insert({prod_tags_t::TERM, term_productions});
}

ast_node_t* parser_t::match_program(parser_t *p) {
    return nullptr;
}

ast_node_t* parser_t::match_decls_1(parser_t* p) {
    return nullptr;
}

ast_node_t* parser_t::match_decls_2(parser_t* p) {
    return nullptr;
}

ast_node_t* parser_t::match_decl_1(parser_t* p) {
    return nullptr;
}

ast_node_t* parser_t::match_decl_2(parser_t* p) {
    return nullptr;
}

ast_node_t* parser_t::match_stmt_block(parser_t* p) {
    return nullptr;
}

ast_node_t* parser_t::match_stmt_if(parser_t* p) {
    return nullptr;
}

ast_node_t* parser_t::match_stmt_decl(parser_t* p) {
    return nullptr;
}

ast_node_t* parser_t::match_stmt_assign(parser_t* p) {
    return nullptr;
}

ast_node_t* parser_t::match_stmts_1(parser_t* p) {
    return nullptr;
}

ast_node_t* parser_t::match_stmts_2(parser_t* p) {
    return nullptr;
}

ast_node_t* parser_t::match_expr_arithop(parser_t* p) {
    return nullptr;
}

ast_node_t* parser_t::match_expr_negated(parser_t* p) {
    return nullptr;
}

ast_node_t* parser_t::match_expr_relop(parser_t* p) {
    return nullptr;
}

ast_node_t* parser_t::match_expr_term(parser_t* p) {
    return nullptr;
}

ast_node_t* parser_t::match_arithop_plus(parser_t* p) {
    return nullptr;
}

ast_node_t* parser_t::match_arithop_minus(parser_t* p) {
    return nullptr;
}

ast_node_t* parser_t::match_relop_equals(parser_t* p) {
    return nullptr;
}

ast_node_t* parser_t::match_relop_not_equals(parser_t* p) {
    return nullptr;
}

ast_node_t* parser_t::match_term_identifier(parser_t* p) {
    return nullptr;
}

ast_node_t* parser_t::match_term_literal(parser_t* p) {
    return nullptr;
}
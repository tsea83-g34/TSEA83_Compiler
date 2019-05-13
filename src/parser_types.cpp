
#include "../include/parser_types.h"
#include "../include/parser.h"
#include "../include/translator.h"
#include "../include/symbol_table.h"

#include "../include/instructions.h"
#include "../include/helper_functions.h"

#include <stack>
#include <iostream>
#include <vector>

void program_t::undo(parser_t* p) {
    decls->undo(p);
    delete decls;
}

std::string program_t::get_string(parser_t* p) {
    return "(program){ " + decls->get_string(p) + " }";
}

void decls_t::undo(parser_t* p) {
    
    if (rest != nullptr) {
        rest->undo(p);
        delete rest;
    }

    first->undo(p);
    delete first;
}

std::string decls_t::get_string(parser_t* p) {
    
    std::string result = first->get_string(p);
    if (rest != nullptr) result += " " + rest->get_string(p);
    return result;
}

void var_decl_t::undo(parser_t* p) {

    // If last token is a semi colon, put it back
    if (tokens.back()->tag == lex::tag_t::SEMI_COLON) {
        p->put_back_token(tokens.back());
        tokens.pop_back();
    }

    if (value != nullptr) {

        value->undo(p);
        delete value;

        // Put back assignment token
        p->put_back_token(tokens.back());
        tokens.pop_back();
    }

    // Put back id token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    if (is_pointer) {
        // Put back * token
        p->put_back_token(tokens.back());
        tokens.pop_back();
    }

    // Put back type token
    p->put_back_token(tokens.back());
    tokens.pop_back();

}

void simple_array_decl_t::undo(parser_t* p) {

    // Put back ; token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    // Put back ] token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    size->undo(p);
    delete size;

    // Put back [ token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    // Put back id token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    // Put back type token
    p->put_back_token(tokens.back());
    tokens.pop_back();
}

std::string simple_array_decl_t::get_string(parser_t* p) {
    return "(array_decl)[ type: " + p->get_type_name(type) + " id: " + identifier + " ]";
}


void init_list_array_decl_t::undo(parser_t* p) {

    // Put back ; token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    // Put back } token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    init_list->undo(p);
    delete init_list;

    // Put back { token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    // Put back = token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    // Put back ] token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    // Put back [ token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    // Put back id token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    // Put back type token
    p->put_back_token(tokens.back());
    tokens.pop_back();
}

std::string init_list_array_decl_t::get_string(parser_t* p) {
    return "(array_decl)[ type: " + p->get_type_name(type) + " id: " + identifier + " ]{ " + init_list->get_string(p) + " }";
}


void str_array_decl_t::undo(parser_t* p) {

    // Put back ; token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    // Put back str_lit token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    // Put back = token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    // Put back ] token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    // Put back [ token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    // Put back id token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    // Put back type "char" token
    p->put_back_token(tokens.back());
    tokens.pop_back();
}

std::string str_array_decl_t::get_string(parser_t* p) {
    return "(array_decl)[ type: " + p->get_type_name(type) + " id: " + identifier + "]{ " + string_literal + " }";
}

std::string var_decl_t::get_string(parser_t* p) {
    std::string ptr_str = (is_pointer) ? "* " : "";

    return "(var_decl)[ " + p->get_type_name(type) + " " + ptr_str + id + " ]" +
        ((value != nullptr) ? "{ " + value->get_string(p) + " }" : "");
}

void func_decl_t::undo(parser_t* p) {
    
    if (stmt != nullptr) {
        stmt->undo(p);
        delete stmt;
    } else {
        // Put back ; token
        p->put_back_token(tokens.back());
        tokens.pop_back();
    }

    // Put back ) token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    // If the declaration has parameters, undo them
    if (param_list != nullptr) {
        param_list->undo(p);
        delete param_list;
    }

    // Put back ( token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    // Put back id token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    // Put back type token
    p->put_back_token(tokens.back());
    tokens.pop_back();

}

std::string func_decl_t::get_string(parser_t* p) {

    std::string stmt_string   = (stmt != nullptr) ? stmt->get_string(p) : "";
    std::string params_string = (param_list != nullptr) ? (" params: " + param_list->get_string(p)) : "";

    return "(function)[type: " + p->get_type_name(type) + " id: " + id + params_string + "]{" + stmt_string + "}";
}

void param_decls_t::undo(parser_t* p) {

    if (rest != nullptr) {
        rest->undo(p);
        delete rest;
    }

    first->undo(p);
    delete first;
}

std::string param_decls_t::get_string(parser_t* p) {
    std::string result = first->get_string(p);
    if (rest != nullptr) result += " " + rest->get_string(p);
    return result;
}

void asm_params_t::undo(parser_t* p) {

    if (rest != nullptr) {
        rest->undo(p);
        delete rest;
    }

    first->undo(p);
    delete first;
}

void init_list_t::undo(parser_t* p) {

    if (rest != nullptr) {
        rest->undo(p);
        delete rest;
    }

    first->undo(p);
    delete first;
}

std::string init_list_t::get_string(parser_t* p) {
    std::string result = first->get_string(p);
    if (rest != nullptr) result += " " + rest->get_string(p);
    return result;
}

std::string asm_params_t::get_string(parser_t* p) {
    std::string result = first->get_string(p);
    if (rest != nullptr) result += " " + rest->get_string(p);
    return result;
}

void param_decl_t::undo(parser_t* p) {

    // Put back id token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    if (is_pointer) {
        // Put back * token
        p->put_back_token(tokens.back());
        tokens.pop_back();
    }

    // Put back type token
    p->put_back_token(tokens.back());
    tokens.pop_back();
}

std::string param_decl_t::get_string(parser_t* p) {
    std::string ptr_str = (is_pointer) ? "* " : "";
    return p->get_type_name(type) + " " + ptr_str + id;
}

void params_t::undo(parser_t* p) {

    if (rest != nullptr) {
        rest->undo(p);
        delete rest;
    }

    first->undo(p);
    delete first;
}

std::string params_t::get_string(parser_t* p) {
    std::string result = first->get_string(p);
    if (rest != nullptr) result += " " + rest->get_string(p);
    return result;
}

void stmts_t::undo(parser_t* p) {

    if (rest != nullptr) {
        rest->undo(p);
        delete rest;
    }

    first->undo(p);
    delete first;
}

std::string stmts_t::get_string(parser_t* p) {
    std::string result = first->get_string(p);
    if (rest != nullptr) result += " " + rest->get_string(p);
    return result;
}

void block_stmt_t::undo(parser_t* p) {
    
    // Put back } token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    if (statements != nullptr) {
        statements->undo(p);
        delete statements;
    }

    // Put back { token
    p->put_back_token(tokens.back());
    tokens.pop_back();
}

std::string block_stmt_t::get_string(parser_t* p) {
    return "{ " + ((statements != nullptr) ? statements->get_string(p) : " ") + " }";
}

void if_stmt_t::undo(parser_t* p) {

    // If there was an else statement
    if (else_actions != nullptr) {
        else_actions->undo(p);
        delete else_actions;

        // Put back else token
        p->put_back_token(tokens.back());
        tokens.pop_back();
    }
    
    if (actions != nullptr) {
        actions->undo(p);
        delete actions;
    }

    // Put back ) token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    cond->undo(p);
    delete cond;

    // Put back ( token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    // Put back if token
    p->put_back_token(tokens.back());
    tokens.pop_back();
}

std::string if_stmt_t::get_string(parser_t* p) {
    std::string else_str = (else_actions) ? " (else){ " + else_actions->get_string(p) + " }" : "";
    return "(if)[ cond{ " + cond->get_string(p) + " } ]{ " + actions->get_string(p) + " }" + else_str;
}

void while_stmt_t::undo(parser_t* p) {
    
    if (actions != nullptr) {
        actions->undo(p);
        delete actions;
    }

    // Put back ) token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    cond->undo(p);
    delete cond;

    // Put back ( token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    // Put back if token
    p->put_back_token(tokens.back());
    tokens.pop_back();
}

std::string while_stmt_t::get_string(parser_t* p) {
    return "(while)[ cond{ " + cond->get_string(p) + " } ]{ " + actions->get_string(p) + " }";
}

void asm_stmt_t::undo(parser_t* p) {
    
    // Put back ) token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    params->undo(p);
    delete params;

    // Put back string literal token
    p->put_back_token(tokens.back());
    tokens.pop_back();    

    // Put back ( token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    // Put back asm token
    p->put_back_token(tokens.back());
    tokens.pop_back();
}

std::string asm_stmt_t::get_string(parser_t* p) {
    return "(asm){ " + literal + " " + params->get_string(p) + " }";
}

void assignment_stmt_t::undo(parser_t* p) {
    
    // Put back ; token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    rvalue->undo(p);
    delete rvalue;

    // Put back = token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    // Put back id token
    p->put_back_token(tokens.back());
    tokens.pop_back();
}

std::string assignment_stmt_t::get_string(parser_t* p) {
    return "(assign)[ " + identifier + " value( " + rvalue->get_string(p) + " )]";
}

void deref_assignment_stmt_t::undo(parser_t* p) {
    
    // Put back ; token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    rvalue->undo(p);
    delete rvalue;

    // Put back = token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    // Put back id token
    p->put_back_token(tokens.back());
    tokens.pop_back(); 

    // Put back * token
    p->put_back_token(tokens.back());
    tokens.pop_back();
}

std::string deref_assignment_stmt_t::get_string(parser_t* p) {
    return "(assign)[ *" + identifier + " value( " + rvalue->get_string(p) + " )]";
}

void indexed_assignment_stmt_t::undo(parser_t* p) {
    
    // Put back ; token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    rvalue->undo(p);
    delete rvalue;

    // Put back = token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    // Put back ] token
    p->put_back_token(tokens.back());
    tokens.pop_back(); 

    index->undo(p);
    delete index;

    // Put back [ token
    p->put_back_token(tokens.back());
    tokens.pop_back(); 

    // Put back id token
    p->put_back_token(tokens.back());
    tokens.pop_back(); 

}

std::string indexed_assignment_stmt_t::get_string(parser_t* p) {
    return "(assign)[ " + identifier + " [ " + index->get_string(p) + " ] value( " + rvalue->get_string(p) + " )]";
}

void return_stmt_t::undo(parser_t* p) {

    // Put back ; token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    return_value->undo(p);
    delete return_value;

    // Put back return token
    p->put_back_token(tokens.back());
    tokens.pop_back();
}

std::string return_stmt_t::get_string(parser_t* p) {
    return "(return)[ " + return_value->get_string(p) + " ]";
}

void expr_stmt_t::undo(parser_t* p) {
    
    // Put ; return token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    e->undo(p);
    delete e;
}

std::string expr_stmt_t::get_string(parser_t* p) {
    return "(expr)[ " + e->get_string(p) + " ]";
}

void neg_expr_t::undo(parser_t* p) {
    
    value->undo(p);
    delete value;
    
    // Put back - token
    p->put_back_token(tokens.back());
    tokens.pop_back();
}

std::string neg_expr_t::get_string(parser_t* p) {
    return "- " + value->get_string(p);
}

void not_expr_t::undo(parser_t* p) {
    
    value->undo(p);
    delete value;
    
    // Put back ! token
    p->put_back_token(tokens.back());
    tokens.pop_back();
}

std::string not_expr_t::get_string(parser_t* p) {
    return "!" + value->get_string(p);
}

void deref_term_t::undo(parser_t* p) {

    // Put back id token
    p->put_back_token(tokens.back());
    tokens.pop_back();    
    
    // Put back * token
    p->put_back_token(tokens.back());
    tokens.pop_back();
}

std::string deref_term_t::get_string(parser_t* p) {
    return "*" + identifier;
}

void indexed_term_t::undo(parser_t* p) {

    // Put back ] token
    p->put_back_token(tokens.back());
    tokens.pop_back();    
    
    index->undo(p);
    delete index;

    // Put back [ token
    p->put_back_token(tokens.back());
    tokens.pop_back();   

    // Put back id token
    p->put_back_token(tokens.back());
    tokens.pop_back();
}

std::string indexed_term_t::get_string(parser_t* p) {
    return identifier + "[ " + index->get_string(p) + " ]";
}

void term_expr_t::undo(parser_t* p) {
    
    t->undo(p);
    delete t;

}

std::string term_expr_t::get_string(parser_t* p) {
    return t->get_string(p);
}

void term_t::undo(parser_t* p) {
    
    // Put back id or literal token
    p->put_back_token(tokens.back());
    tokens.pop_back();

}

std::string id_term_t::get_string(parser_t* p) {
    return identifier;
}

std::string lit_term_t::get_string(parser_t* p) {
    return std::to_string(literal);
}

void call_term_t::undo(parser_t* p) {
    
    // Put back ) token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    // If parameters are given, return them
    if (params != nullptr) {
        params->undo(p);
        delete params;
    }

    // Put back ( token
    p->put_back_token(tokens.back());
    tokens.pop_back();
    
    // Put back id token
    p->put_back_token(tokens.back());
    tokens.pop_back();
    
}

std::string call_term_t::get_string(parser_t* p) {
    std::string params_string = (params != nullptr) ? params->get_string(p) : "";
    return function_identifier + "(" + params_string + ")";
}

void expr_term_t::undo(parser_t* p) {
    
    // Put back ) token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    // put back expression
    expr->undo(p);
    delete expr;

    // Put back ( token
    p->put_back_token(tokens.back());
    tokens.pop_back();
    
}

std::string expr_term_t::get_string(parser_t* p) {
    return "(" + expr->get_string(p) + ")";
}

void addr_of_term_t::undo(parser_t* p) {
    
    // Put back identifier token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    // Put back & token
    p->put_back_token(tokens.back());
    tokens.pop_back();
    
}

std::string addr_of_term_t::get_string(parser_t* p) {
    return "( *" + identifier + ")";
}


void binop_expr_t::undo(parser_t* p) {

    // Put back the rest of the expression

    if (left_assoc) {
        term->undo(p);
    } else {
        rest->undo(p);
    }

    // Put back operator token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    // Put back term
    if (left_assoc) {
        rest->undo(p);
    } else {
        term->undo(p);
    }

}

std::string add_binop_t::get_string(parser_t* p) {
    if (left_assoc) {
        return "(" + rest->get_string(p) + ") + " + term->get_string(p);
    } else {
        return term->get_string(p) + " + (" + rest->get_string(p) + ")";
    }
}

std::string sub_binop_t::get_string(parser_t* p) {
    if (left_assoc) {
        return "(" + rest->get_string(p) + ") - " + term->get_string(p);
    } else {
        return term->get_string(p) + " - (" + rest->get_string(p) + ")";
    }
}

std::string and_binop_t::get_string(parser_t* p) {
    if (left_assoc) {
        return "(" + rest->get_string(p) + ") & " + term->get_string(p);
    } else {
        return term->get_string(p) + " & (" + rest->get_string(p) + ")";
    }
}

std::string or_binop_t::get_string(parser_t* p) {
    if (left_assoc) {
        return "(" + rest->get_string(p) + ") | " + term->get_string(p);
    } else {
        return term->get_string(p) + " | (" + rest->get_string(p) + ")";
    }
}

std::string mult_binop_t::get_string(parser_t* p) {
    if (left_assoc) {
        return "(" + rest->get_string(p) + ") * " + term->get_string(p);
    } else {
        return term->get_string(p) + " * (" + rest->get_string(p) + ")";
    }
}

std::string eq_binop_t::get_string(parser_t* p) {
    if (left_assoc) {
        return "(" + rest->get_string(p) + ") == " + term->get_string(p);
    } else {
        return term->get_string(p) + " == (" + rest->get_string(p) + ")";
    }
}

std::string neq_binop_t::get_string(parser_t* p) {
    if (left_assoc) {
        return "(" + rest->get_string(p) + ") != " + term->get_string(p);
    } else {
        return term->get_string(p) + " != (" + rest->get_string(p) + ")";
    }
}

std::string less_binop_t::get_string(parser_t* p) {
    if (left_assoc) {
        return "(" + rest->get_string(p) + ") < " + term->get_string(p);
    } else {
        return term->get_string(p) + " < (" + rest->get_string(p) + ")";
    }
}

std::string greater_binop_t::get_string(parser_t* p) {
    if (left_assoc) {
        return "(" + rest->get_string(p) + ") > " + term->get_string(p);
    } else {
        return term->get_string(p) + " > (" + rest->get_string(p) + ")";
    }
}

std::string less_eq_binop_t::get_string(parser_t* p) {
    if (left_assoc) {
        return "(" + rest->get_string(p) + ") <= " + term->get_string(p);
    } else {
        return term->get_string(p) + " <= (" + rest->get_string(p) + ")";
    }
}

std::string greater_eq_binop_t::get_string(parser_t* p) {
    if (left_assoc) {
        return "(" + rest->get_string(p) + ") >= " + term->get_string(p);
    } else {
        return term->get_string(p) + " >= (" + rest->get_string(p) + ")";
    }
}

// -------------------- EVALUATION ---------------------
//
// -----------------------------------------------------

bool neg_expr_t::evaluate(int* result) {
    
    int val;
    bool success = value->evaluate(&val);

    if (!success) return false;

    *result = - val;
    return true;
}

bool not_expr_t::evaluate(int* result) {
    
    int val;
    bool success = value->evaluate(&val);

    if (!success) return false;

    *result = !val;
    return true;
}

bool deref_term_t::evaluate(int* result) {
    return false;
}

bool term_expr_t::evaluate(int* result) {
    return t->evaluate(result);
}

bool lit_term_t::evaluate(int* result) {
    *result = literal;
    return true;
}

bool call_term_t::evaluate(int* result) {
    return false;
}

bool id_term_t::evaluate(int* result) {
    return false;
}

bool addr_of_term_t::evaluate(int* result) {
    return false;
}

bool indexed_term_t::evaluate(int* result) {
    return false;
}

bool expr_term_t::evaluate(int* result) {
    
    return expr->evaluate(result);
}

bool add_binop_t::evaluate(int* result) {
    // Assumes left associativity
    int rest_val;
    int term_val;

    bool success;
    success = rest->evaluate(&rest_val) && term->evaluate(&term_val);

    if (!success) return false;

    *result = rest_val + term_val;
    return true;
}

bool sub_binop_t::evaluate(int* result) {
    // Assumes left associativity
    int rest_val;
    int term_val;

    bool success;
    success = rest->evaluate(&rest_val) && term->evaluate(&term_val);

    if (!success) return false;

    *result = rest_val - term_val;
    return true;
}

bool and_binop_t::evaluate(int* result) {
    // Assumes left associativity
    int rest_val;
    int term_val;

    bool success;
    success = rest->evaluate(&rest_val) && term->evaluate(&term_val);

    if (!success) return false;

    *result = rest_val & term_val;
    return true;
}

bool or_binop_t::evaluate(int* result) {
    // Assumes left associativity
    int rest_val;
    int term_val;

    bool success;
    success = rest->evaluate(&rest_val) && term->evaluate(&term_val);

    if (!success) return false;

    *result = rest_val | term_val;
    return true;
}

bool mult_binop_t::evaluate(int* result) {
    // Assumes left associativity
    int rest_val;
    int term_val;

    bool success;
    success = rest->evaluate(&rest_val) && term->evaluate(&term_val);

    if (!success) return false;

    *result = rest_val * term_val;
    return true;
}

bool eq_binop_t::evaluate(int* result) {
    // Assumes left associativity
    int rest_val;
    int term_val;

    bool success;
    success = rest->evaluate(&rest_val) && term->evaluate(&term_val);

    if (!success) return false;

    *result = rest_val == term_val;
    return true;
}

bool neq_binop_t::evaluate(int* result) {
    // Assumes left associativity
    int rest_val;
    int term_val;

    bool success;
    success = rest->evaluate(&rest_val) && term->evaluate(&term_val);

    if (!success) return false;

    *result = rest_val != term_val;
    return true;
}

bool less_binop_t::evaluate(int* result) {
    // Assumes left associativity
    int rest_val;
    int term_val;

    bool success;
    success = rest->evaluate(&rest_val) && term->evaluate(&term_val);

    if (!success) return false;

    *result = rest_val < term_val;
    return true;
}

bool greater_binop_t::evaluate(int* result) {
    // Assumes left associativity
    int rest_val;
    int term_val;

    bool success;
    success = rest->evaluate(&rest_val) && term->evaluate(&term_val);

    if (!success) return false;

    *result = rest_val > term_val;
    return true;
}

bool less_eq_binop_t::evaluate(int* result) {
    // Assumes left associativity
    int rest_val;
    int term_val;

    bool success;
    success = rest->evaluate(&rest_val) && term->evaluate(&term_val);

    if (!success) return false;

    *result = rest_val <= term_val;
    return true;
}

bool greater_eq_binop_t::evaluate(int* result) {
    // Assumes left associativity
    int rest_val;
    int term_val;

    bool success;
    success = rest->evaluate(&rest_val) && term->evaluate(&term_val);

    if (!success) return false;

    *result = rest_val >= term_val;
    return true;
}

// ------------------ BINOP REWRITING ------------------
//
// -----------------------------------------------------


add_binop_t* add_binop_t::duplicate() {
    
    add_binop_t* result = new add_binop_t();
    
    // Give ownership of the operator token
    result->tokens.push_back(tokens.back());
    tokens.pop_back();
    
    return result;
}

sub_binop_t* sub_binop_t::duplicate() {
    
    sub_binop_t* result = new sub_binop_t();
    
    // Give ownership of the operator token
    result->tokens.push_back(tokens.back());
    tokens.pop_back();
    
    return result;
}

mult_binop_t* mult_binop_t::duplicate() {
    
    mult_binop_t* result = new mult_binop_t();
    
    // Give ownership of the operator token
    result->tokens.push_back(tokens.back());
    tokens.pop_back();
    
    return result;
}

and_binop_t* and_binop_t::duplicate() {
    
    and_binop_t* result = new and_binop_t();
    
    // Give ownership of the operator token
    result->tokens.push_back(tokens.back());
    tokens.pop_back();
    
    return result;
}

or_binop_t* or_binop_t::duplicate() {
    
    or_binop_t* result = new or_binop_t();
    
    // Give ownership of the operator token
    result->tokens.push_back(tokens.back());
    tokens.pop_back();
    
    return result;
}

eq_binop_t* eq_binop_t::duplicate() {
        
    eq_binop_t* result = new eq_binop_t();
    
    // Give ownership of the operator token
    result->tokens.push_back(tokens.back());
    tokens.pop_back();
    
    return result;
}

neq_binop_t* neq_binop_t::duplicate() {
        
    neq_binop_t* result = new neq_binop_t();
    
    // Give ownership of the operator token
    result->tokens.push_back(tokens.back());
    tokens.pop_back();
    
    return result;
}

less_binop_t* less_binop_t::duplicate() {
        
    less_binop_t* result = new less_binop_t();
    
    // Give ownership of the operator token
    result->tokens.push_back(tokens.back());
    tokens.pop_back();
    
    return result;
}

greater_binop_t* greater_binop_t::duplicate() {
        
    greater_binop_t* result = new greater_binop_t();
    
    // Give ownership of the operator token
    result->tokens.push_back(tokens.back());
    tokens.pop_back();
    
    return result;
}

less_eq_binop_t* less_eq_binop_t::duplicate() {
        
    less_eq_binop_t* result = new less_eq_binop_t();
    
    // Give ownership of the operator token
    result->tokens.push_back(tokens.back());
    tokens.pop_back();
    
    return result;
}

greater_eq_binop_t* greater_eq_binop_t::duplicate() {
        
    greater_eq_binop_t* result = new greater_eq_binop_t();
    
    // Give ownership of the operator token
    result->tokens.push_back(tokens.back());
    tokens.pop_back();
    
    return result;
}

expr_t* binop_expr_t::rewrite(expr_t* e) {
    
    binop_expr_t* starting_root = dynamic_cast<binop_expr_t*>(e);

    // If given expression is not a binary operation, return
    if (starting_root == nullptr) return e;

    // If the given binop tree is already left associative, return
    if (starting_root->left_assoc) return e;

    std::stack<binop_expr_t*> op_stack{};
    op_stack.push(starting_root);


    // Build up the stack
    int i = 0;
    while (true) {
        i++;
        binop_expr_t* current = op_stack.top();
        binop_expr_t* rest = dynamic_cast<binop_expr_t*>(current->rest);

        // If cannot cast to binary operation, we have reached the bottom
        if (rest == nullptr) break;

        op_stack.push(rest);
    }

    // Set up variables that will be used

    // The root of the resulting tree
    binop_expr_t* new_tree = nullptr;

    // The current binop in the new tree
    binop_expr_t* current = nullptr;

    // The previous binop in the new tree, the parent of current
    binop_expr_t* previous_new = nullptr;

    // The previous binop in the old tree
    binop_expr_t* previous_old = nullptr;

    while (!op_stack.empty()) {
        
        // If this is the first node in the new tree
        if (new_tree == nullptr) {

            // Duplicate the node, set the term to the term of the old node
            // and set associativity to left
            new_tree = op_stack.top()->duplicate();

            new_tree->term = op_stack.top()->rest;
            new_tree->left_assoc = true;
            current = new_tree;
            previous_old = op_stack.top();
            op_stack.pop();
            continue;
        }

        // Get a new operation from the stack and duplicate it
        previous_new = current;
        current = op_stack.top()->duplicate();
        
        // Set the associativity of new node to left, and it's term to the term of the previous operation from the old tree
        current->left_assoc = true;

        // If the rest was a tree within parenthesis, rewrite it
        expr_term_t* term_tree = dynamic_cast<expr_term_t*>(previous_old->term);
        if (term_tree != nullptr) {
            term_tree->expr = rewrite(term_tree->expr);
        }
        
        current->term = previous_old->term;


        // Set the rest of the previous node from the new tree to this node
        previous_new->rest = current;

        // Delete the previous node from the old tree and update it's value
        delete previous_old;
        previous_old = op_stack.top();
        op_stack.pop();
    }

    // When the stack has been emptied set the rest of the current node in the new tree
    // to the term of the term of the last old node
    current->left_assoc = true;
    current->rest = previous_old->term;
    delete previous_old;
    return new_tree;
}


// -------------------- TRANSLATION --------------------
//
// -----------------------------------------------------

int program_t::translate(translator_t* t) {

    decls->translate(t);

}

int decls_t::translate(translator_t* t) {
    
    first->translate(t);

    if (rest != nullptr) rest->translate(t);

    return 0;
}

int func_decl_t::translate(translator_t* t) {
    
    std::cout << "Translating function declaration..." << std::endl;

    // If a function with that name already exists
    func_info_t* potential_old_function = t->symbol_table.get_func(id);
    func_info_t* current_function = new func_info_t(this, t);

    if (potential_old_function != nullptr) {

        // If the existing function with the same name as this have the same signature
        if ((*potential_old_function) != (*current_function)) {
            delete current_function;
            throw translation_error("Mismatching declarations of function \"" + id + "\"");
        }
        
        // If it is defined and this is also a definition we have multiple definitions
        if (potential_old_function->defined && stmt != nullptr) {
            delete current_function;
            throw translation_error("Multiple definition of function \"" + id + "\"");
        }
        
        // If it is only declared
            
        // If this is also just a declaration, we are done here.
        if (stmt == nullptr) {
            delete current_function;
            return 0;
        }

        // Otherwise remove previous declaration
        t->symbol_table.remove_func(id);
    }
    
    t->symbol_table.add_func(id, current_function);

    if (stmt == nullptr) return 0;

    current_function->defined = true;

    print_label(t, current_function->identifier);

    move_instr(t, BASE_POINTER, STACK_POINTER);

    t->symbol_table.push_scope(true);

    if (param_list != nullptr) {
        // Starts at 2 to accomodate for return address pointer
        current_function->params_size = 2;
        param_list->translate(t, current_function, 0);
    }
    
    stmt->translate(t);

    // Set return register to 0 and print ret instruction
    if (!t->last_was_ret) {
        move_instr(t, RETURN_REGISTER, NULL_REGISTER);
        ret_instr(t);
    }

    // Free the registers containing local variables in the current scope
    t->reg_alloc.free_scope(t->symbol_table.get_current_scope(), true);
    
    t->symbol_table.pop_scope();

    t->print_instruction_row("", false, false);
}

int param_decls_t::translate(translator_t* t, func_info_t* f, int param_index) {
    
    first->translate(t, f, param_index);
    if (rest != nullptr) {
        
        rest->translate(t, f, param_index + 1);

    } else {
        // remove return pointer offset from params_size
        f->params_size -= 2;

        /*
        // If this is the last param, align the stack to 4
        if (f->params_size % 4 != 0) {
                f->params_size += 4 - f->params_size % 4;
        }*/

    }
    
}

int param_decl_t::translate(translator_t* t, func_info_t* f, int param_index) {

    // If a parameter with that name already exists
    if (t->symbol_table.get_current_scope()->at(id)) {
        throw translation_error("A parameter with name \"" + id + "\" already exists!");
    }
    
    var_info_t* param_info = &f->param_vector[param_index];
    
    // Create address structure for parameter
    int base_offset = static_cast<local_addr_info_t*>(param_info->address)->base_offset;
    
    local_addr_info_t* addr = new local_addr_info_t(base_offset);

    // Size is zero since allocation happens on function call, right now, we simply
    // want to add the variables to the namespace
    var_info_t* var = t->symbol_table.add_var(id, type, 0, addr);
    var->is_pointer = is_pointer;
    
    // Allocate a register for the variable and load it
    t->reg_alloc.allocate(var, true, false);
}

int params_t::translate(translator_t* t, func_info_t* func, int param_index) {
    
    // Push params backwards
    if (rest != nullptr) rest->translate(t, func, param_index + 1);

    std::stringstream output;
    int first_value = 0;
    bool first_evaluated = first->evaluate(&first_value);

    if (first_evaluated) {
        
        // Add temporary variable to scope to allow register allocation
        var_info_t* var;
        int reg = allocate_temp_imm(t, "__param__", first_value, &var);

        int alignment = func->get_alignment(param_index);

        if (alignment) {
            std::cout << "Pushing alignment " << alignment << std::endl;
            subi_instr(t, STACK_POINTER, STACK_POINTER, alignment);
            t->symbol_table.get_current_scope()->push(alignment);
        }

        int size = (func->param_vector[param_index].is_pointer) ? POINTER_SIZE : t->type_table.at(func->param_vector[param_index].type)->size;
        if (func->param_vector[param_index].is_pointer) std::cout << "PUSHING POINTER" << std::endl;
        push_instr(t, reg, size);

        t->reg_alloc.free(reg);
        t->symbol_table.get_current_scope()->remove(var->name);
        delete var;

    } else {

        int reg = first->translate(t);

        int alignment = func->get_alignment(param_index);

        if (alignment) {
            subi_instr(t, STACK_POINTER, STACK_POINTER, alignment);
            t->symbol_table.get_current_scope()->push(alignment);
        }

        // Size = 2 if pointer sizeof(type) otherwise
        int size = (func->param_vector[param_index].is_pointer) ? POINTER_SIZE : t->type_table.at(func->param_vector[param_index].type)->size;
        push_instr(t, reg, size);

    }

    return -1;
}

int asm_params_t::translate(translator_t*) {
    
} 

int var_decl_t::translate(translator_t* t) {
    
    if (t->symbol_table.is_global_scope()) {
        
        std::cout << "Found global variable declaration!" << std::endl;
        
        if (t->symbol_table.get_current_scope()->at(id)) {
            throw translation_error("Multiple definition of global symbol \"" + id + "\" At " + std::to_string(tokens.front()->line_number) + ":" + std::to_string(tokens.front()->column_number));
        }

        type_descriptor_t* type_desc = t->type_table.at(type);
        
        global_addr_info_t* addr = new global_addr_info_t(id);

        // Size is zero since the variable is allocated statically
        var_info_t* var = t->symbol_table.add_var(id, type, 0, addr);
        var->is_pointer = is_pointer;
        
        int size = (is_pointer) ? POINTER_SIZE : type_desc->size;

        // Global variable
        if (value == nullptr) {
            t->static_alloc(id, size, 0);
            return 0;    
        };

        int constant_value = 0;
        bool evaluated = value->evaluate(&constant_value);

        if (evaluated) {
            t->static_alloc(id, size, constant_value);
            return 0;
        }

        // Allocate space for the variable
        t->static_alloc(id, size, 0);

        t->set_data_mode(true);

        // If not evaluated translate the expression
        int value_reg = value->translate(t);
        bool was_temp = t->reg_alloc.is_temporary(value_reg);

        // If value was a function return value, allocate a register
        if (value_reg == RETURN_REGISTER) {
            
            int reg = t->reg_alloc.allocate(var, false, false);
            move_instr(t, reg, value_reg);
            t->reg_alloc.touch(reg, true);
            t->reg_alloc.free(reg);

        // If it was another register give ownership of the register to the new variable
        } else {

            var_info_t* temp_info = t->reg_alloc.give_ownership(value_reg, var);
            t->reg_alloc.touch(value_reg, true);
            t->reg_alloc.free(value_reg);
            
            // Remove and deallocate the temporary variable
            if (was_temp) {
                std::cout << "Removing " << temp_info->name << std::endl;
                t->symbol_table.get_current_scope()->remove(temp_info->name);
                delete temp_info;
            }
        }

        t->set_data_mode(false);
    } else {
        
        std::cout << "Found local variable!" << std::endl;

        // Local variable

        if (t->symbol_table.get_current_scope()->at(id)) {
            throw translation_error("Multiple definition of local symbol \"" + id + "\"");
        }

        // Try to evaluate the expression
        int constant_value = 0;
        bool is_constant = false;
        if (value != nullptr) is_constant = value->evaluate(&constant_value);

        // Get type descriptor of the type of the variable
        type_descriptor_t* type_desc = t->type_table.at(type);
        int size = (is_pointer) ? POINTER_SIZE : type_desc->size;
 
        std::cout << "Type: " << type_desc->name << " size: " << size << std::endl;

        // Acquire current scope
        scope_t* current_scope = t->symbol_table.get_current_scope();
        
        // Calculate the offset of the variables adress from the base pointer
        // TODO: currently allocates 4 bytes for every type, type_desc->size
        
        int variable_base_offset = 0;
        int size_to_allocate = 0;
        int alignment = 0;

        if (size <= 2) {
            // If the variable size is 2 or less, allocate 2 bytes
            size_to_allocate = 2;
        } else {
            // If the variable size is more than 2 align the stack to 4 and allocate a multiple of 4 bytes
            alignment = current_scope->align(4);

            // If the size is already a multiple of 4, allocate that many bytes, otherwise round it up to the nearest multiple
            size_to_allocate = (size % 4 == 0) ? size : size + (4 - size % 4);
        }

        variable_base_offset = -(current_scope->get_end_offset() + size_to_allocate);
        
        local_addr_info_t* addr = new local_addr_info_t(variable_base_offset);
        var_info_t* var = t->symbol_table.add_var(id, type, size_to_allocate, addr);

        // Allocate memory for the variable
        subi_instr(t, STACK_POINTER, STACK_POINTER, alignment + size_to_allocate);

        // If a value was given, load it into a register
        if (value != nullptr) {

            if (is_constant) {
                // Allocate register, but dont load value since we are solving that locally
                int register_index = t->reg_alloc.allocate(var, false, false);
                
                load_immediate(t, register_index, constant_value);
            } else {
                
                int value_reg = value->translate(t);
                bool was_temp = t->reg_alloc.is_temporary(value_reg);

                // If value was a function return value, allocate a register
                if (value_reg == RETURN_REGISTER) {
                    
                    int reg = t->reg_alloc.allocate(var, false, false);
                    move_instr(t, reg, value_reg);
                    t->reg_alloc.touch(reg, true);

                // If it was another register give ownership of the register to the new variable
                } else {

                    var_info_t* temp_info = t->reg_alloc.give_ownership(value_reg, var);
                    t->reg_alloc.touch(value_reg, true);
                    
                    // Remove and deallocate the temporary variable
                    if (was_temp) {
                        std::cout << "Removing " << temp_info->name << std::endl;
                        t->symbol_table.get_current_scope()->remove(temp_info->name);
                        delete temp_info;
                    }
                }
            }
        } 
    }
}

int simple_array_decl_t::translate(translator_t* t) {
    
    type_descriptor_t* type_desc = t->type_table.at(type);
    int element_size = type_desc->size;
    
    int array_size;
    size->evaluate(&array_size);

    if (t->symbol_table.is_global_scope()) {

        std::cout << "Found global array declaration!" << std::endl;
        
        if (t->symbol_table.get_current_scope()->at(identifier)) {
            throw translation_error("Multiple definition of global symbol \"" + identifier + "\" At " + std::to_string(tokens.front()->line_number) + ":" + std::to_string(tokens.front()->column_number));
        }

        global_addr_info_t* addr = new global_addr_info_t(identifier);

        // Size is zero since the variable is allocated statically
        var_info_t* var = t->symbol_table.add_var(identifier, type, 0, addr);
        var->is_pointer = true;

        t->static_alloc_array(identifier, element_size, array_size);

    } else {

        std::cout << "Found local array declaration!" << std::endl;

        // Local variable

        if (t->symbol_table.get_current_scope()->at(identifier)) {
            throw translation_error("Multiple definition of local symbol \"" + identifier + "\"");
        }

        scope_t* current_scope = t->symbol_table.get_current_scope();

        int variable_base_offset = 0;
        
        // Align the scope to the size of the array
        int pre_alignment = current_scope->align(element_size);
        
        // Push the size of the array to the stack
        current_scope->push(element_size * array_size);

        variable_base_offset = - current_scope->get_end_offset();

        // Align the stack to two afterwards
        int post_alignment = current_scope->align(2);

        local_addr_info_t* addr = new local_addr_info_t(variable_base_offset);
        var_info_t* var = t->symbol_table.add_var(identifier, type, 0, addr);
        var->is_pointer = true;

        int total_stack_size = pre_alignment + element_size * array_size + post_alignment;
        subi_instr(t, STACK_POINTER, STACK_POINTER, total_stack_size);
    
    }
}

int init_list_array_decl_t::translate(translator_t* t) {

    // Acquire values form initializer list
    std::vector<int> values;
    init_list_to_vector(init_list, values);

    type_descriptor_t* type_desc = t->type_table.at(type);
    int element_size = type_desc->size;
    
    int array_size = values.size();

    if (t->symbol_table.is_global_scope()) {

        std::cout << "Found global initializer list array declaration!" << std::endl;
        
        if (t->symbol_table.get_current_scope()->at(identifier)) {
            throw translation_error("Multiple definition of global symbol \"" + identifier + "\" At " + std::to_string(tokens.front()->line_number) + ":" + std::to_string(tokens.front()->column_number));
        }

        global_addr_info_t* addr = new global_addr_info_t(identifier);

        // Size is zero since the variable is allocated statically
        var_info_t* var = t->symbol_table.add_var(identifier, type, 0, addr);
        var->is_pointer = true;

        t->static_alloc_array_init(identifier, element_size, values);
    
    } else {

        scope_t* current_scope = t->symbol_table.get_current_scope();

        int variable_base_offset = 0;
        
        // Align the scope to the size of the array
        int pre_alignment = current_scope->align(element_size);

        if (pre_alignment) subi_instr(t, STACK_POINTER, STACK_POINTER, pre_alignment);

        var_info_t* temp_var;
        int reg = allocate_temp(t, "__temp__", &temp_var);
        
        for (int i = array_size - 1; i >= 0; i--) {

            load_immediate(t, reg, values[i]);
            push_instr(t, reg, element_size);

        }

        t->reg_alloc.touch(reg, false);
        t->reg_alloc.free(temp_var, false);

        variable_base_offset = - current_scope->get_end_offset();

        // Align the stack to two afterwards
        int post_alignment = current_scope->align(2);

        if (post_alignment) subi_instr(t, STACK_POINTER, STACK_POINTER, post_alignment);

        local_addr_info_t* addr = new local_addr_info_t(variable_base_offset);
        var_info_t* var = t->symbol_table.add_var(identifier, type, 0, addr);
        var->is_pointer = true;

    }
}

int str_array_decl_t::translate(translator_t* t) {

    if (t->type_table.at(type)->name != "char") {
        throw translation_error("Allocating string literal to an array with type other than char " + std::to_string(tokens.front()->line_number) + ":" + std::to_string(tokens.front()->column_number)); 
    }
    
    type_descriptor_t* type_desc = t->type_table.at(type);
    int element_size = type_desc->size;

    str_lit_to_str(string_literal, string_literal);
    
    // -2 for quotation marks and +1 for null character
    int array_size = string_literal.size() - 2 + 1;
    
    if (t->symbol_table.is_global_scope()) {

        std::cout << "Found global string array declaration!" << std::endl;
        
        if (t->symbol_table.get_current_scope()->at(identifier)) {
            throw translation_error("Multiple definition of global symbol \"" + identifier + "\" At " + std::to_string(tokens.front()->line_number) + ":" + std::to_string(tokens.front()->column_number));
        }

        global_addr_info_t* addr = new global_addr_info_t(identifier);

        // Size is zero since the variable is allocated statically
        var_info_t* var = t->symbol_table.add_var(identifier, type, 0, addr);
        var->is_pointer = true;

        t->static_alloc_array_str(identifier, string_literal);

    } else {

        scope_t* current_scope = t->symbol_table.get_current_scope();

        int variable_base_offset = 0;

        var_info_t* temp_var;
        int reg = allocate_temp(t, "__temp__", &temp_var);
        
        load_immediate(t, reg, 0);
        push_instr(t, reg, element_size);

        for (int i = string_literal.size() - 2; i >= 1; i--) {

            load_immediate(t, reg, string_literal[i]);
            push_instr(t, reg, element_size);

        }

        t->reg_alloc.touch(reg, false);
        t->reg_alloc.free(temp_var, false);

        variable_base_offset = - current_scope->get_end_offset();

        // Align the stack to two afterwards
        int post_alignment = current_scope->align(2);

        if (post_alignment) subi_instr(t, STACK_POINTER, STACK_POINTER, post_alignment);

        local_addr_info_t* addr = new local_addr_info_t(variable_base_offset);
        var_info_t* var = t->symbol_table.add_var(identifier, type, 0, addr);
        var->is_pointer = true;
    }
}

int stmts_t::translate(translator_t* t) {
    
    first->translate(t);

    if (rest != nullptr) rest->translate(t);

}

int block_stmt_t::translate(translator_t* t) {
    
    t->symbol_table.push_scope(true);

    if (statements != nullptr) statements->translate(t);
    
    // Throw away variables
    int scope_size = t->symbol_table.get_current_scope()->size();

    // If the scope has not returned pop scope variables
    if (!t->last_was_ret && scope_size != 0) {
        addi_instr(t, STACK_POINTER, STACK_POINTER, scope_size);
    }

    // Free the registers containing local variables in the current scope
    t->reg_alloc.free_scope(t->symbol_table.get_current_scope());
    
    t->symbol_table.pop_scope();
}

int if_stmt_t::translate(translator_t* t) {

    int constant_value = 0;
    bool cond_evaluated = cond->evaluate(&constant_value);

    if (cond_evaluated) {
        
        if (constant_value) {
            // If constant evaluates to true, translate statements
            actions->translate(t);    
        } else {
            // If constant evaluates to true, translate else statements
            if (else_actions) else_actions->translate(t);   
        } 

        return -1;

    } else {

        int cond_reg = cond->translate(t);
        std::string else_label = t->label_allocator.get_label_name();
        std::string end_label = t->label_allocator.get_label_name();

        t->reg_alloc.store_context();
        
        // Test if true or false
        cmpi_instr(t, cond_reg, 1);

        branch_instr(t, BRNE_INSTR, else_label);

        actions->translate(t);

        t->reg_alloc.store_context();
        
        branch_instr(t, JMP_INSTR, end_label);

        print_label(t, else_label);

        if (else_actions != nullptr) else_actions->translate(t);

        t->reg_alloc.store_context();

        print_label(t, end_label);
    }
}

int while_stmt_t::translate(translator_t* t) {

    int constant_value = 0;
    bool cond_evaluated = cond->evaluate(&constant_value);

    if (cond_evaluated) {
        
        if (!constant_value) return -1;

        std::string start_label = t->label_allocator.get_label_name();
        std::string end_label   = t->label_allocator.get_label_name();

        t->reg_alloc.store_context();

        print_label(t, start_label);

        actions->translate(t);

        t->reg_alloc.store_context();

        branch_instr(t, JMP_INSTR, start_label);

        print_label(t, end_label);

    } else {

        std::string start_label = t->label_allocator.get_label_name();
        std::string end_label   = t->label_allocator.get_label_name();

        t->reg_alloc.store_context();

        print_label(t, start_label);

        int cond_reg = cond->translate(t);

        cmpi_instr(t, cond_reg, 1);

        branch_instr(t, BRNE_INSTR, end_label);

        actions->translate(t);

        t->reg_alloc.store_context();

        branch_instr(t, JMP_INSTR, start_label);

        print_label(t, end_label);

    }
}

int asm_stmt_t::translate(translator_t* t) {

    std::vector<term_t*> param_vector;
    std::vector<int> temp_registers;

    asm_params_t* current = params;
    while (current != nullptr) {
        param_vector.push_back(dynamic_cast<term_t*>(current->first));
        current = current->rest;
    }

    int first_reg = -1;
    std::string result = literal; 
    for (term_t* term : param_vector) {
        
        int reg;
        // If the current parameter is a literal, load it into a register
        if (dynamic_cast<lit_term_t*>(term)) {
            
            var_info_t* var;
            int value = 0;
            term->evaluate(&value);

            reg = allocate_temp_imm(t, "__temp__", value, &var);
            temp_registers.push_back(reg);

        } else {

            reg = term->translate(t);

        }
        if (first_reg == -1) first_reg = reg;
        result.replace(result.find("$"), 1, get_register_string(t, reg));
    }

    if (result.find("$") != result.npos) {
        throw translation_error("Mismatched number of \'$\' and parameters");
    }

    t->reg_alloc.touch(first_reg, true);

    t->print_instruction_row(result, true, false);
    
    // Free temporary registers
    for (int reg : temp_registers) {
        if (reg == first_reg) throw translation_error("Inline assembly: First operand should not be a temporary value");
        t->reg_alloc.free(reg);
    }
}

int assignment_stmt_t::translate(translator_t* t) {

    var_info_t* var = t->symbol_table.get_var(identifier);

    int constant_value = 0;
    bool value_evaluated = rvalue->evaluate(&constant_value);

    if (value_evaluated) {

        int reg = t->reg_alloc.allocate(var, false, true);
        load_immediate(t, reg, constant_value);
        t->reg_alloc.touch(reg, true);

    } else {
        
        int right_register = rvalue->translate(t);

        if (t->reg_alloc.is_temporary(right_register)) {
            // If right value is temporary, take ownership of the register
            
            // If the variable is already stored in a register, deallocate that register, without storing
            t->reg_alloc.free(var, false);

            var_info_t* temp_var = t->reg_alloc.give_ownership(right_register, var);
            t->reg_alloc.touch(right_register, true);

            // Remove old variable
            // t->symbol_table.get_current_scope()->remove(temp_var->name);
            // delete temp_var;

        } else {

            // If it is not temporary, allocate a register and move
            // Could this steal the register of the variable being loaded?
            int reg = t->reg_alloc.allocate(var, false, false);
            t->reg_alloc.touch(reg, true);

            move_instr(t, reg, right_register);
        }
    }
}

int deref_assignment_stmt_t::translate(translator_t* t) {

    var_info_t* var = t->symbol_table.get_var(identifier);
    int var_size = t->type_table.at(var->type)->size;

    if (!var->is_pointer) std::cout << "-- Translator warning: Dereferencing non-pointer variable " << var->name << " " << tokens.front()->line_number << ":" << tokens.front()->column_number << std::endl;

    int constant_value = 0;
    bool value_evaluated = rvalue->evaluate(&constant_value);
    int ptr_reg = t->reg_alloc.allocate(var, true, false);

    if (value_evaluated) {


        var_info_t* temp_var;
        int const_reg = allocate_temp_imm(t, "__temp__", constant_value, &temp_var);
        
        store_instr(t, ptr_reg, const_reg, nullptr, var_size);
        t->reg_alloc.free(temp_var, false);

    } else {
        
        int right_register = rvalue->translate(t);

        if (right_register == ptr_reg) {
            ptr_reg = t->reg_alloc.allocate(var, true, false);
        }

        store_instr(t, ptr_reg, right_register, nullptr, var_size);
    }
}

int indexed_assignment_stmt_t::translate(translator_t* t) {

    var_info_t* var = t->symbol_table.get_var(identifier);
    int var_size = t->type_table.at(var->type)->size;

    if (!var->is_pointer) std::cout << "-- Translator warning: Dereferencing non-pointer variable " << var->name << " " << tokens.front()->line_number << ":" << tokens.front()->column_number << std::endl;

    int constant_value = 0;
    bool value_evaluated = rvalue->evaluate(&constant_value);
    int ptr_reg = t->reg_alloc.allocate(var, true, false);

    int constant_index = 0;
    bool index_evaluated = index->evaluate(&constant_index);

    var_info_t* ptr_temp = give_ownership_temp(t, "__temp__", ptr_reg);
    
    if (index_evaluated && constant_index < std::numeric_limits<unsigned short>().max()) {
        
        addi_instr(t, ptr_reg, ptr_reg, var_size * constant_index);

    } else {
        var_info_t* size_const_var;
        
        int index_register = index->translate(t);
        int size_const_reg = allocate_temp_imm(t, "__temp__", var_size, &size_const_var);

        mult_instr(t, index_register, index_register, size_const_reg);
        add_instr(t, ptr_reg, ptr_reg, index_register);

        t->reg_alloc.free(size_const_var, false);
    }

    if (value_evaluated) {

        var_info_t* temp_var;
        int const_reg = allocate_temp_imm(t, "__temp__", constant_value, &temp_var);
        
        store_instr(t, ptr_reg, const_reg, nullptr, var_size);
        t->reg_alloc.free(temp_var, false);

    } else {
        
        int right_register = rvalue->translate(t);

        if (right_register == ptr_reg) {
            ptr_reg = t->reg_alloc.allocate(var, true, false);
        }

        store_instr(t, ptr_reg, right_register, nullptr, var_size);
    }
    t->reg_alloc.free(ptr_temp, false);
}


int return_stmt_t::translate(translator_t* t) {
    
    int constant_value = 0;
    bool value_evaluated = return_value->evaluate(&constant_value);

    if (value_evaluated) {
        
        load_immediate(t, RETURN_REGISTER, constant_value);

    } else {

        int value_reg = return_value->translate(t);

        if (value_reg != RETURN_REGISTER) {
            move_instr(t, RETURN_REGISTER, value_reg);
        }

    }
    int total_scope_size = t->symbol_table.get_current_scope()->get_end_offset();

    // Pop whole scope variables
    if (total_scope_size != 0) addi_instr(t, STACK_POINTER, STACK_POINTER, total_scope_size);

    ret_instr(t);
}

int expr_stmt_t::translate(translator_t* t) {
    
    return e->translate(t);

}

int neg_expr_t::translate(translator_t* t) {
    
    std::stringstream output;

    int reg = value->translate(t);
    std::string register_string = t->reg_alloc.get_register_string(reg);
    
     // If the allocated register is not temporary, take ownership of it
    if (!t->reg_alloc.is_temporary(reg) && reg != RETURN_REGISTER) {
        
        give_ownership_temp(t, "__temp__", reg);

    }
    
    // Print neg instr
    neg_instr(t, reg, reg);
    return reg;
}

int not_expr_t::translate(translator_t* t) {
    
    std::stringstream output;

    int reg = value->translate(t);
    std::string register_string = t->reg_alloc.get_register_string(reg);
    
     // If the allocated register is not temporary, take ownership of it
    if (!t->reg_alloc.is_temporary(reg) && reg != RETURN_REGISTER) {
        
        give_ownership_temp(t, "__temp__", reg);

    }
    
    // Print not instr
    not_instr(t, reg, reg);
    return reg;
}

int term_expr_t::translate(translator_t* t) {
    
    return this->t->translate(t);
}

int id_term_t::translate(translator_t* t) {
    
    // Find variable, allocate register, return index
    var_info_t* var = t->symbol_table.get_var(identifier);

    // Load variable into a register, not marked as temporary
    int register_index = t->reg_alloc.allocate(var, true, false);

    // Return the register index of the allocated register
    return register_index;
}

int addr_of_term_t::translate(translator_t* t) {

    var_info_t* var = t->symbol_table.get_var(identifier);

    bool is_local = dynamic_cast<local_addr_info_t*>(var->address) != nullptr;
    bool is_global = dynamic_cast<global_addr_info_t*>(var->address) != nullptr;

    std::string temp_name = t->name_allocator.get_name("__temp__");

    // Add temporary variable to scope to allow register allocation
    var_info_t* temp_var = t->symbol_table.add_var(temp_name, 0, 0, nullptr);

    int reg = t->reg_alloc.allocate(temp_var, false, false);

    if (is_local) {
        
        local_addr_info_t* l_addr = dynamic_cast<local_addr_info_t*>(var->address);

        addi_instr(t, reg, BASE_POINTER, l_addr->base_offset);
    
    } else { // Is global

        global_addr_info_t* g_addr = dynamic_cast<global_addr_info_t*>(var->address);

        tri_operand_imm_str_instr(t, ADD_IMM_INSTR, reg, NULL_REGISTER, g_addr->label);

    }

    return reg;
}

int deref_term_t::translate(translator_t* t) {

    var_info_t* var = t->symbol_table.get_var(identifier);
    int var_size = t->type_table.at(var->type)->size;

    if (!var->is_pointer) std::cout << "-- Translator warning: Dereferencing non-pointer variable " << var->name << " " << tokens.front()->line_number << ":" << tokens.front()->column_number << std::endl;

    bool is_local = dynamic_cast<local_addr_info_t*>(var->address) != nullptr;
    bool is_global = dynamic_cast<global_addr_info_t*>(var->address) != nullptr;

    std::string temp_name = t->name_allocator.get_name("__temp__");

    // Add temporary variable to scope to allow register allocation
    var_info_t* temp_var = t->symbol_table.add_var(temp_name, 0, 0, nullptr);

    int reg = t->reg_alloc.allocate(var, false, false);

    // Take ownership of register
    t->reg_alloc.give_ownership(reg, temp_var);

    // Load the value pointed to by reg into reg with offset 0
    load_instr(t, reg, reg, nullptr, var_size);

    return reg;
}

int indexed_term_t::translate(translator_t* t) {

    var_info_t* var = t->symbol_table.get_var(identifier);
    int var_size = t->type_table.at(var->type)->size;

    if (!var->is_pointer) std::cout << "-- Translator warning: Dereferencing non-pointer variable " << var->name << " " << tokens.front()->line_number << ":" << tokens.front()->column_number << std::endl;

    bool is_local = dynamic_cast<local_addr_info_t*>(var->address) != nullptr;
    bool is_global = dynamic_cast<global_addr_info_t*>(var->address) != nullptr;

    std::string temp_name = t->name_allocator.get_name("__temp__");

    // Add temporary variable to scope to allow register allocation
    var_info_t* temp_var = t->symbol_table.add_var(temp_name, 0, 0, nullptr);

    int reg = t->reg_alloc.allocate(var, true, false);

    int constant_index = 0;
    bool index_evaluated = index->evaluate(&constant_index);

    if (index_evaluated) {

        int temp_reg = t->reg_alloc.allocate(temp_var, false, false);
        load_instr(t, temp_reg, reg, constant_index * var_size, var_size);
        return temp_reg;

    } else {

        int index_reg = index->translate(t);

        index_reg = take_ownership_or_allocate(t, "__temp__", index_reg);
        
        add_instr(t, index_reg, reg, index_reg);
        load_instr(t, index_reg, index_reg, nullptr, var_size);

        return index_reg;
    }
}

int call_term_t::translate(translator_t* t) {
    
    // TODO: Fix alignment?

    func_info_t* func = t->symbol_table.get_func(function_identifier);

    if (func == nullptr) throw translation_error("Function " + function_identifier + " is not declared. " + std::to_string(tokens.front()->line_number) + ":" + std::to_string(tokens.front()->column_number));

    scope_t* current_scope = t->symbol_table.get_current_scope();
    
    int context_size = current_scope->get_end_offset() + 2;//(func->param_vector.size()) ? 2 : 0;
    std::cout << "Context size: " << context_size << std::endl;
    
    int alignment = (context_size % 4) ? 4 - (context_size % 4) : 0;
    std::cout << "Alignment: " << alignment << std::endl;

    // Push base pointer to stack
    push_instr(t, BASE_POINTER, POINTER_SIZE);

    // Align the stack to 4
    current_scope->push(alignment);
    if (alignment) {
        subi_instr(t, STACK_POINTER, STACK_POINTER, alignment);
    }

    // Push parameters to stack
    if (params != nullptr) params->translate(t, func, 0);

    // Store current context
    t->reg_alloc.store_context();

    // If the parameters are 4 aligned, stack wont be because of return pointer, offset it with 2 unless there are no parameters
    if (func->param_vector.size() && func->total_stack_size % 4 == 0) {
        alignment += 2;
        current_scope->push(2);
        subi_instr(t, STACK_POINTER, STACK_POINTER, 2);
    }

    // Call function
    call_instr(t, function_identifier);

    // Pop parameters and alignment
    current_scope->pop(alignment + func->total_stack_size);

    // Pop parameters
    if (alignment + func->total_stack_size) addi_instr(t, STACK_POINTER, STACK_POINTER, func->total_stack_size + alignment);

    // Pop base pointer from stack
    pop_instr(t, BASE_POINTER, POINTER_SIZE);

    return RETURN_REGISTER; 
}

int lit_term_t::translate(translator_t* t) {
    
}

int expr_term_t::translate(translator_t* t) {
    
    return expr->translate(t);
    
}

int add_binop_t::translate(translator_t* t) {
    return translate_binop_imm(t, this, ADD_INSTR, ADD_IMM_INSTR);
}

int sub_binop_t::translate(translator_t* t) {
    return translate_binop_imm(t, this, SUB_INSTR, SUB_IMM_INSTR);
}

int and_binop_t::translate(translator_t* t) {
    return translate_binop(t, this, AND_INSTR);
}

int or_binop_t::translate(translator_t* t) {
    return translate_binop(t, this, OR_INSTR);
}

int mult_binop_t::translate(translator_t* t) {
    return translate_binop(t, this, MULT_INSTR);
}


int eq_binop_t::translate(translator_t* t) {
    return translate_binop_relational(t, this, BREQ_INSTR);
}

int neq_binop_t::translate(translator_t* t) {
    return translate_binop_relational(t, this, BRNE_INSTR);
}

int less_binop_t::translate(translator_t* t) {
    return translate_binop_relational(t, this, BRLT_INSTR);
}

int greater_binop_t::translate(translator_t* t) {
    return translate_binop_relational(t, this, BRGT_INSTR);
}

int less_eq_binop_t::translate(translator_t* t) {
    return translate_binop_relational(t, this, BRLE_INSTR);
}

int greater_eq_binop_t::translate(translator_t* t) {
    return translate_binop_relational(t, this, BRGE_INSTR);
}
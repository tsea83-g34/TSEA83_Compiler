
#include "../include/parser_types.h"
#include "../include/parser.h"
#include "../include/translator.h"

#include <stack>

void program_t::undo(parser_t* p) {
    decls->undo(p);
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

    // Put back type token
    p->put_back_token(tokens.back());
    tokens.pop_back();

}

std::string var_decl_t::get_string(parser_t* p) {
    return "(var_decl)[ " + p->get_type_name(type) + " " + id + " ]" +
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

void param_decl_t::undo(parser_t* p) {

    // Put back id token
    p->put_back_token(tokens.back());
    tokens.pop_back();

    // Put back type token
    p->put_back_token(tokens.back());
    tokens.pop_back();
}

std::string param_decl_t::get_string(parser_t* p) {
    return p->get_type_name(type) + " " + id;
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
    return "{ " + statements->get_string(p) + " }";
}

void if_stmt_t::undo(parser_t* p) {
    
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
    return "(if)[ cond{ " + cond->get_string(p) + " } ]{ " + actions->get_string(p) + " }";
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

void arith_expr_t::undo(parser_t* p) {
    
    right->undo(p);
    op->undo(p);
    left->undo(p);

    delete right;
    delete op;
    delete left;
}

std::string arith_expr_t::get_string(parser_t* p) {
    return left->get_string(p) + " " + op->get_string(p) + " " + right->get_string(p);
}

void rel_expr_t::undo(parser_t* p) {
    
    right->undo(p);
    op->undo(p);
    left->undo(p);
    
    delete right;
    delete op;
    delete left;

}

std::string rel_expr_t::get_string(parser_t* p) {
    return left->get_string(p) + " " + op->get_string(p) + " " + right->get_string(p);
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

void arithop_t::undo(parser_t* p) {
    
    // Put back + or - token
    p->put_back_token(tokens.back());
    tokens.pop_back();

}

void relop_t::undo(parser_t* p) {
    
    // Put back == or != token
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

std::string arithop_plus_t::get_string(parser_t* p) {
    return std::string("+");
}

std::string arithop_minus_t::get_string(parser_t* p) {
    return std::string("-");
}

std::string relop_equals_t::get_string(parser_t* p) {
    return std::string("==");
}

std::string relop_not_equals_t::get_string(parser_t* p) {
    return std::string("!=");
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

// -------------------- EVALUATION ---------------------
//
// -----------------------------------------------------

bool arith_expr_t::evaluate(int* result) {
    return op->evaluate(result, left, right);
}

bool rel_expr_t::evaluate(int* result) {
    return op->evaluate(result, left, right);
}

bool neg_expr_t::evaluate(int* result) {
    
    int val;
    bool success = value->evaluate(&val);

    if (!success) return false;

    *result = - val;
    return true;
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

bool arithop_plus_t::evaluate(int* result, term_t* left, expr_t* right) {

    int left_val;
    int right_val;

    bool success;
    success = left->evaluate(&left_val) && right->evaluate(&right_val);

    if (!success) return false;

    *result = left_val + right_val;
    return true; 
}

bool arithop_minus_t::evaluate(int* result, term_t* left, expr_t* right) {

    int left_val;
    int right_val;

    bool success;
    success = left->evaluate(&left_val) && right->evaluate(&right_val);

    if (!success) return false;

    *result = left_val - right_val;
    return true; 
}

bool relop_equals_t::evaluate(int* result, term_t* left, expr_t* right) {

    int left_val;
    int right_val;

    bool success;
    success = left->evaluate(&left_val) && right->evaluate(&right_val);

    if (!success) return false;

    *result = (left_val == right_val);
    return true; 
}

bool relop_not_equals_t::evaluate(int* result, term_t* left, expr_t* right) {
    int left_val;
    int right_val;

    bool success;
    success = left->evaluate(&left_val) && right->evaluate(&right_val);

    if (!success) return false;

    *result = (left_val != right_val);
    return true; 
}

bool add_binop_t::evaluate(int* result) {
    return false;
}

bool sub_binop_t::evaluate(int* result) {
    return false;
}

bool eq_binop_t::evaluate(int* result) {
    return false;
}

bool neq_binop_t::evaluate(int* result) {
    return false;
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
    
}

int param_decls_t::translate(translator_t* t) {
    
}

int param_decl_t::translate(translator_t* t) {
    
}

int params_t::translate(translator_t* t) {
    
}

int var_decl_t::translate(translator_t* t) {
    
    if (t->symbol_table.is_global_scope()) {
        
        // Global variable
        char buffer[100];
        memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "%s:\n", id.c_str());
        
        int constant_value;
        value->evaluate(&constant_value);

        type_descriptor_t* type_desc = t->type_table.at(type);
        t->print_instruction_row(t->static_alloc(type_desc->size, constant_value));
    } else {
        // Local variable (stack)
    }
}

int stmts_t::translate(translator_t* t) {
    
}

int block_stmt_t::translate(translator_t* t) {
    
}

int if_stmt_t::translate(translator_t* t) {
    
}

int assignment_stmt_t::translate(translator_t* t) {
    
}

int return_stmt_t::translate(translator_t* t) {
    
}

int expr_stmt_t::translate(translator_t* t) {
    
}

int arith_expr_t::translate(translator_t* t) {
    
}

int rel_expr_t::translate(translator_t* t) {
    
}

int neg_expr_t::translate(translator_t* t) {
    
}

int term_expr_t::translate(translator_t* t) {
    
}

int id_term_t::translate(translator_t* t) {
    
}

int call_term_t::translate(translator_t* t) {
    
}

int lit_term_t::translate(translator_t* t) {
    
}

int arithop_plus_t::translate(translator_t* t) {
    
}

int arithop_minus_t::translate(translator_t* t) {
    
}

int relop_equals_t::translate(translator_t* t) {
    
}

int relop_not_equals_t::translate(translator_t* t) {
    
}

int add_binop_t::translate(translator_t* t) {
    
}

int sub_binop_t::translate(translator_t* t) {
    
}

int eq_binop_t::translate(translator_t* t) {
    
}

int neq_binop_t::translate(translator_t* t) {
    
}






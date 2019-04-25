
#include "../include/parser_types.h"
#include "../include/parser.h"
#include "../include/translator.h"
#include "../include/symbol_table.h"

#include <stack>
#include <iostream>

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
    
    // TODO: Add scope with parameters
    func_info_t* info = new func_info_t(this, t);

    if (stmt == nullptr) return 0;

    std::string label = info->identifier + ":";
    
    t->print_instruction_row(label, false);
    
    stmt->translate(t);

    t->print_instruction_row("ret", true);
}

int param_decls_t::translate(translator_t* t, func_info_t* f) {
    
    first->translate(t, f);

    if (rest != nullptr) rest->translate(t, f);
}

int param_decl_t::translate(translator_t* t, func_info_t* f) {
    
    // Starts at 2 to accomodate for return address pointer
    int current_base_offset = 2;
}

int params_t::translate(translator_t* t) {
    
}

int var_decl_t::translate(translator_t* t) {
    
    if (t->symbol_table.is_global_scope()) {
        
        std::cout << "Found global variable!" << std::endl;
        
        // Global variable
        
        int constant_value = 0;
        if (value != nullptr) value->evaluate(&constant_value);

        type_descriptor_t* type_desc = t->type_table.at(type);
        
        global_addr_info_t* addr = new global_addr_info_t(id);
        t->symbol_table.add_var(id, type, type_desc->size, addr);

        t->static_alloc(id, type_desc->size, constant_value);

    } else {
        
        std::cout << "Found local variable!" << std::endl;

        // Local variable

        int constant_value = 0;
        if (value != nullptr) value->evaluate(&constant_value);

        // Get type descriptor of the type of the variable
        type_descriptor_t* type_desc = t->type_table.at(type);

        std::cout << "Type: " << type_desc->name << " size: " << type_desc->size << std::endl;

        // Acquire current scope
        scope_t* current_scope = t->symbol_table.get_current_scope();
        
        // Calculate the offset of the variables adress from the base pointer
        // TODO: currently allocates 4 bytes for every type, type_desc->size
        
        int variable_base_offset = 0;
        int size_to_allocate = 0;
        int alignment = 0;

        if (type_desc->size <= 2) {
            // If the variable size is 2 or less, allocate 2 bytes
            size_to_allocate = 2;
        } else {
            // If the variable size is more than 2 align the stack to 4 and allocate a multiple of 4 bytes
            alignment = current_scope->align(4);

            // If the size is already a multiple of 4, allocate that many bytes, otherwise round it up to the nearest multiple
            size_to_allocate = (type_desc->size % 4 == 0) ? type_desc->size : type_desc->size + (4 - type_desc->size % 4);

            std::cout << "size to alloc: " << size_to_allocate << std::endl;
            std::cout << "Alignment: " << alignment << std::endl;
        }

        variable_base_offset = -(current_scope->get_end_offset() + size_to_allocate);
        
        local_addr_info_t* addr = new local_addr_info_t(variable_base_offset);
        var_info_t* var = t->symbol_table.add_var(id, type, size_to_allocate, addr);

        // Print instructions
        std::stringstream output;
        if (value != nullptr) {
            int register_index = t->reg_alloc.allocate(var, true);
            
            if (size_to_allocate == 4) {
                
                int hi = (constant_value & 0xFFFF0000) >> 16;
                output << "movhi ";
                output << t->reg_alloc.get_register_string(register_index);
                output << ", " << hi;
                t->print_instruction_row(output.str(), true);
                output = std::stringstream();
            }

            int lo = constant_value & 0x0000FFFF;
            output << "movlo ";
            output << t->reg_alloc.get_register_string(register_index);
            output << ", " << lo;
            t->print_instruction_row(output.str(), true);
            output = std::stringstream();

            if (alignment) { 
                output << "subi " << "SP, " << "SP, " << alignment;
                t->print_instruction_row(output.str(), true);
                output = std::stringstream();
            }

            output << "push" << "[" << size_to_allocate << "] ";
            output << t->reg_alloc.get_register_string(register_index);
            t->print_instruction_row(output.str(), true);
            output = std::stringstream();
        
        } else {

            output << "subi " << "SP, " << "SP, " << alignment+size_to_allocate;
            t->print_instruction_row(output.str(), true);

        }
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

    std::stringstream output;
    output << "addi SP, SP, " << scope_size;
    t->print_instruction_row(output.str(), true);
    // --------------------

    t->symbol_table.pop_scope();
}

int if_stmt_t::translate(translator_t* t) {
    
}

int assignment_stmt_t::translate(translator_t* t) {
    
}

int return_stmt_t::translate(translator_t* t) {
    
}

int expr_stmt_t::translate(translator_t* t) {
    
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

int add_binop_t::translate(translator_t* t) {
    
}

int sub_binop_t::translate(translator_t* t) {
    
}

int eq_binop_t::translate(translator_t* t) {
    
}

int neq_binop_t::translate(translator_t* t) {
    
}






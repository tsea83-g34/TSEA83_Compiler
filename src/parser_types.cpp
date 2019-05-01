
#include "../include/parser_types.h"
#include "../include/parser.h"
#include "../include/translator.h"
#include "../include/symbol_table.h"

#include "../include/instructions.h"
#include "../include/helper_functions.h"

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
    return "{ " + ((statements != nullptr) ? statements->get_string(p) : " ") + " }";
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

    t->symbol_table.push_scope(false);

    if (param_list != nullptr) {
        // Starts at 2 to accomodate for return address pointer
        current_function->params_size = 2;
        param_list->translate(t, current_function);
    }
    
    stmt->translate(t);

    // Set return register to 0 and print ret instruction
    if (!t->last_was_ret) {
        move_instr(t, RETURN_REGISTER, NULL_REGISTER);
        ret_instr(t);
    }

    // Free the registers containing local variables in the current scope
    t->reg_alloc.free_scope(t->symbol_table.get_current_scope());
    
    t->symbol_table.pop_scope();
}

int param_decls_t::translate(translator_t* t, func_info_t* f) {
    
    first->translate(t, f);
    if (rest != nullptr) {
        
        rest->translate(t, f);

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

int param_decl_t::translate(translator_t* t, func_info_t* f) {

    // If a parameter with that name already exists
    if (t->symbol_table.get_current_scope()->at(id)) {
        throw translation_error("A parameter with name \"" + id + "\" already exists!");
    }
    
    int size = t->type_table.at(type)->size;

    // If the current base offset is mis-aligned in relation to the size of this parameter
    // adjust it 
    // TODO: This isn't accounted for...
    if (f->params_size % size != 0) {
        f->params_size += (-f->params_size) % size;
    }
    
    // Create address structure for parameter
    local_addr_info_t* addr = new local_addr_info_t(f->params_size);

    // Size is zero since allocation happens on function call, right now, we simply
    // want to add the variables to the namespace
    var_info_t* var = t->symbol_table.add_var(id, type, 0, addr);
    
    // Allocate a register for the variable and load it
    t->reg_alloc.allocate(var, true, false);

    // Update parameter size with the size of the new variable
    f->params_size += size;
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

        push_instr(t, reg, t->type_table.at(func->param_vector[param_index].type)->size);

        t->reg_alloc.free(reg);
        t->symbol_table.get_current_scope()->remove(var->name);
        delete var;

    } else {

        int reg = first->translate(t);

        push_instr(t, reg, t->type_table.at(func->param_vector[param_index].type)->size);

    }

    return -1;
}

int var_decl_t::translate(translator_t* t) {
    
    if (t->symbol_table.is_global_scope()) {
        
        std::cout << "Found global variable declaration!" << std::endl;
        
        if (t->symbol_table.get_current_scope()->at(id)) {
            throw translation_error("Multiple definition of global symbol \"" + id + "\"");
        }

        // Global variable
        
        int constant_value = 0;
        if (value != nullptr) value->evaluate(&constant_value);

        type_descriptor_t* type_desc = t->type_table.at(type);
        
        global_addr_info_t* addr = new global_addr_info_t(id);

        // Size is zero since the variable is allocated statically
        t->symbol_table.add_var(id, type, 0, addr);

        t->static_alloc(id, type_desc->size, constant_value);

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
        } else return -1; // Otherwise return

    } else {

        int cond_reg = cond->translate(t);
        std::string end_label = t->label_allocator.get_label_name();

        // Test if true or false
        cmpi_instr(t, cond_reg, 1);

        branch_instr(t, BRNE_INSTR, end_label);

        actions->translate(t);

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
            t->symbol_table.get_current_scope()->remove(temp_var->name);
            delete temp_var;

        } else {

            // If it is not temporary, allocate a register and move
            // Could this steal the register of the variable being loaded?
            int reg = t->reg_alloc.allocate(var, false, true);
            t->reg_alloc.touch(reg, true);

            move_instr(t, reg, right_register);
        }
    }
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

int call_term_t::translate(translator_t* t) {
    
    // TODO: Fix alignment?

    func_info_t* func = t->symbol_table.get_func(function_identifier);

    scope_t* current_scope = t->symbol_table.get_current_scope();
    
    int context_size = current_scope->get_end_offset() + 2 + func->params_size;
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

    // Call function
    call_instr(t, function_identifier);

    // Pop parameters and alignment
    current_scope->pop(alignment + func->params_size);

    // Pop parameters
    std::cout << "Parameters size: " << func->params_size << std::endl;
    if (func->params_size + alignment) addi_instr(t, STACK_POINTER, STACK_POINTER, func->params_size + alignment);

    // Pop base pointer from stack
    pop_instr(t, BASE_POINTER, POINTER_SIZE);

    return RETURN_REGISTER; 
}

int lit_term_t::translate(translator_t* t) {
    
}

int add_binop_t::translate(translator_t* t) {

    // Assume left associativity
    if (!left_assoc) throw translation_error("Expression is right associative");
    
    int left_value = 0;
    bool left_success = rest->evaluate(&left_value);

    int right_value = 0;
    bool right_success = term->evaluate(&right_value);

    int left_register;
    int right_register;


    if (left_success) {

        // Means left was a constant, allocate a register and load the immediate value into it
        var_info_t* var_info;
        left_register = allocate_temp_imm(t, "__temp__", left_value, &var_info);

    } else {

        left_register = rest->translate(t);

        // If the allocated register is not temporary, take ownership of it
        left_register = take_ownership_or_allocate(t, "__temp__", left_register);

    }

    if (right_success) {
        // If right value is larger than 16 bits
        if (right_value > std::numeric_limits<int16_t>().max()) {
            // TODO: This is not very good...
            throw translation_error("Constant cant be larger than 16-bits");
        }

        addi_instr(t, left_register, left_register, right_value);

    } else {

        bool is_function_call = dynamic_cast<call_term_t*>(term) != nullptr;
        var_info_t* var;

        // If term is a function call save temporary value on stack
        if (is_function_call) {
            var = push_temp(t, left_register);
        }

        // Translate term
        right_register = term->translate(t);

        // If term is a function call restore temporary value
        if (is_function_call) {
            left_register = pop_temp(t, var);
        }

        // Print add instruction
        add_instr(t, left_register, left_register, right_register);

    }
    return left_register;
}

int sub_binop_t::translate(translator_t* t) {

    // Assume left associativity
    if (!left_assoc) throw translation_error("Expression is right associative");
    
    int left_value = 0;
    bool left_success = rest->evaluate(&left_value);

    int right_value = 0;
    bool right_success = term->evaluate(&right_value);

    int left_register;
    int right_register;

    if (left_success) {

        // Means left was a constant, allocate a register and load the immediate value into it
        var_info_t* var_info;
        left_register = allocate_temp_imm(t, "__temp__", left_value, &var_info);

    } else {

        left_register = rest->translate(t);

        // If the allocated register is not temporary, take ownership of it
        left_register = take_ownership_or_allocate(t, "__temp__", left_register);
    }

    if (right_success) {
        // If right value is larger than 16 bits
        if (right_value > std::numeric_limits<int16_t>().max()) {
            // TODO: This is not very good...
            throw translation_error("Constant cant be larger than 16-bits");
        }

        // Print sub immediate instruction
        subi_instr(t, left_register, left_register, right_value);

    } else {

        bool is_function_call = dynamic_cast<call_term_t*>(term) != nullptr;
        var_info_t* var;

        // If term is a function call save temporary value on stack
        if (is_function_call) {
            var = push_temp(t, left_register);
        }

        // Translate term
        right_register = term->translate(t);

        // If term is a function call restore temporary value
        if (is_function_call) {
            left_register = pop_temp(t, var);
        }

        // Print sub instruction
        sub_instr(t, left_register, left_register, right_register);

    }
    return left_register;
}

int eq_binop_t::translate(translator_t* t) {
    
    // Assume left associativity
    if (!left_assoc) throw translation_error("Expression is right associative");
    
    int left_value = 0;
    bool left_success = rest->evaluate(&left_value);

    int right_value = 0;
    bool right_success = term->evaluate(&right_value);

    int left_register;
    int right_register;

    if (left_success) {

        // Means left was a constant, allocate a register and load the immediate value into it
        var_info_t* var_info;
        left_register = allocate_temp_imm(t, "__temp__", left_value, &var_info);

    } else {

        left_register = rest->translate(t);

        // If the allocated register is not temporary, take ownership of it
        left_register = take_ownership_or_allocate(t, "__temp__", left_register);
    }

    if (right_success) {
        // If right value is larger than 16 bits
        if (right_value > std::numeric_limits<int16_t>().max()) {
            // TODO: This is not very good...
            throw translation_error("Constant cant be larger than 16-bits");
        }

        // Print cmp immediate instruction
        cmpi_instr(t, left_register, right_value);

    } else {

        bool is_function_call = dynamic_cast<call_term_t*>(term) != nullptr;
        var_info_t* var;

        // If term is a function call save temporary value on stack
        if (is_function_call) {
            var = push_temp(t, left_register);
        }

        // Translate term
        right_register = term->translate(t);

        // If term is a function call restore temporary value
        if (is_function_call) {
            left_register = pop_temp(t, var);
        }

        // Print sub instruction
        cmp_instr(t, left_register, right_register);
    }

    std::string false_label = t->label_allocator.get_label_name();
    std::string end_label = t->label_allocator.get_label_name();
    
    // brne L1
    branch_instr(t, BREQ_INSTR, false_label);

    // addi r, NULL, 1
    addi_instr(t, left_register, NULL_REGISTER, 0);

    // jmp L2
    branch_instr(t, JMP_INSTR, end_label);

    // L1:
    print_label(t, false_label);

    // addi r, NULL, 0
    addi_instr(t, left_register, NULL_REGISTER, 1);

    // L2:
    print_label(t, end_label);

    return left_register;
}

int neq_binop_t::translate(translator_t* t) {
 
    // Assume left associativity
    if (!left_assoc) throw translation_error("Expression is right associative");
    
    int left_value = 0;
    bool left_success = rest->evaluate(&left_value);

    int right_value = 0;
    bool right_success = term->evaluate(&right_value);

    int left_register;
    int right_register;

    if (left_success) {

        // Means left was a constant, allocate a register and load the immediate value into it
        var_info_t* var_info;
        left_register = allocate_temp_imm(t, "__temp__", left_value, &var_info);

    } else {

        left_register = rest->translate(t);

        // If the allocated register is not temporary, take ownership of it
        left_register = take_ownership_or_allocate(t, "__temp__", left_register);
    }

    if (right_success) {
        // If right value is larger than 16 bits
        if (right_value > std::numeric_limits<int16_t>().max()) {
            // TODO: This is not very good...
            throw translation_error("Constant cant be larger than 16-bits");
        }

        // Print cmp immediate instruction
        cmpi_instr(t, left_register, right_value);

    } else {

        bool is_function_call = dynamic_cast<call_term_t*>(term) != nullptr;
        var_info_t* var;

        // If term is a function call save temporary value on stack
        if (is_function_call) {
            var = push_temp(t, left_register);
        }

        // Translate term
        right_register = term->translate(t);

        // If term is a function call restore temporary value
        if (is_function_call) {
            left_register = pop_temp(t, var);
        }

        // Print sub instruction
        cmp_instr(t, left_register, right_register);
    }

    std::string false_label = t->label_allocator.get_label_name();
    std::string end_label = t->label_allocator.get_label_name();
    
    // brne L1
    branch_instr(t, BRNE_INSTR, false_label);

    // addi r, NULL, 1
    addi_instr(t, left_register, NULL_REGISTER, 0);

    // jmp L2
    branch_instr(t, JMP_INSTR, end_label);

    // L1:
    print_label(t, false_label);

    // addi r, NULL, 0
    addi_instr(t, left_register, NULL_REGISTER, 1);

    // L2:
    print_label(t, end_label);

    return left_register;   
}







#include "../include/helper_functions.h"
#include "../include/instructions.h"

#include <sstream>

void load_immediate(translator_t* t, int reg, int value) {
    
    std::stringstream output;
    t->reg_alloc.touch(reg, true);

    if (value < std::numeric_limits<short>().max()) {
        addi_instr(t, reg, NULL_REGISTER, value);
        return;
    }

    int hi = (value & 0xFFFF0000) >> 16;
    movhi_instr(t, reg, hi);
    
    int lo = value & 0x0000FFFF;
    movlo_instr(t, reg, lo);
}

int allocate_temp_imm(translator_t* t, const std::string& name, int value, var_info_t** var) {
    
    std::string temp_name = t->name_allocator.get_name(name);

    // Add temporary variable to scope to allow register allocation
    var_info_t* temp_var = t->symbol_table.add_var(temp_name, 0, 0, nullptr);

    int reg = t->reg_alloc.allocate(temp_var, false, false);

    load_immediate(t, reg, value);

    t->reg_alloc.touch(reg, false);
    
    *var = temp_var;
    return reg;
}

var_info_t* give_ownership_temp(translator_t* t, const std::string& name, int reg) {

    // Add temporary variable to scope to allow register allocation
    std::string left_temp_name = t->name_allocator.get_name(name);
    var_info_t* temp_var = t->symbol_table.add_var(left_temp_name, 0, 0, nullptr);

    t->reg_alloc.give_ownership(reg, temp_var);
    return temp_var;
}

int take_ownership_or_allocate(translator_t* t, const std::string& name, int reg) {
    
    // If the allocated register is not temporary, take ownership of it
    if (!t->reg_alloc.is_temporary(reg) && reg != RETURN_REGISTER) {
        
        give_ownership_temp(t, name, reg);

    } else if (reg == RETURN_REGISTER) {

        // Add temporary variable to scope to allow register allocation
        std::string left_temp_name = t->name_allocator.get_name(name);
        var_info_t* temp_var = t->symbol_table.add_var(left_temp_name, 0, 0, nullptr);
        
        int new_reg = t->reg_alloc.allocate(temp_var, false, false);

        move_instr(t, new_reg, RETURN_REGISTER);
        return new_reg;

    }
    return reg;
}

var_info_t* push_temp(translator_t* t, int reg) {

    var_info_t* var = t->reg_alloc.free(reg);
    int var_size = t->type_table.at(var->type)->size;

    push_instr(t, reg, var_size);
    
    return var;
}

int pop_temp(translator_t* t, var_info_t* var) {

    int reg = t->reg_alloc.allocate(var, false, true);
    int var_size = t->type_table.at(var->type)->size;

    pop_instr(t, reg, var_size);

    return reg;
}

int translate_binop_imm(translator_t*t, binop_expr_t* binop, const std::string& instr, const std::string& imm_instr) {

    // Assume left associativity
    if (!binop->left_assoc) throw translation_error("Expression is right associative");
    
    int left_value = 0;
    bool left_success = binop->rest->evaluate(&left_value);

    int right_value = 0;
    bool right_success = binop->term->evaluate(&right_value);

    int left_register;
    int right_register;


    if (left_success) {

        // Means left was a constant, allocate a register and load the immediate value into it
        var_info_t* var_info;
        left_register = allocate_temp_imm(t, "__temp__", left_value, &var_info);

    } else {

        left_register = binop->rest->translate(t);

        // If the allocated register is not temporary, take ownership of it
        left_register = take_ownership_or_allocate(t, "__temp__", left_register);

    }

    if (right_success) {
        // If right value is larger than 16 bits
        if (right_value > std::numeric_limits<int16_t>().max()) {
            // TODO: This is not very good...
            throw translation_error("Constant cant be larger than 16-bits");
        }

        // Print imm instr
        tri_operand_imm_instr(t, imm_instr, left_register, left_register, right_value);

    } else {

        bool is_function_call = dynamic_cast<call_term_t*>(binop->term) != nullptr;
        var_info_t* var;

        // If term is a function call save temporary value on stack
        if (is_function_call) {
            var = push_temp(t, left_register);
        }

        // Translate term
        right_register = binop->term->translate(t);

        // If term is a function call restore temporary value
        if (is_function_call) {
            left_register = pop_temp(t, var);
        }

        // Print non-imm instruction
        tri_operand_instr(t, instr, left_register, left_register, right_register);

    }
    return left_register;
}

int translate_binop(translator_t* t, binop_expr_t* binop, const std::string& instr) {

    // Assume left associativity
    if (!binop->left_assoc) throw translation_error("Expression is right associative");
    
    int left_value = 0;
    bool left_success = binop->rest->evaluate(&left_value);

    int right_value = 0;
    bool right_success = binop->term->evaluate(&right_value);

    int left_register;
    int right_register;

    if (left_success) {

        // Means left was a constant, allocate a register and load the immediate value into it
        var_info_t* var_info;
        left_register = allocate_temp_imm(t, "__temp__", left_value, &var_info);

    } else {

        left_register = binop->rest->translate(t);

        // If the allocated register is not temporary, take ownership of it
        left_register = take_ownership_or_allocate(t, "__temp__", left_register);
    }

    if (right_success) {

        // Means right was a constant, allocate a register and load the immediate value into it
        var_info_t* var_info;
        right_register = allocate_temp_imm(t, "__temp__", right_value, &var_info);

        t->reg_alloc.free(right_register);
        
    } else {

        bool is_function_call = dynamic_cast<call_term_t*>(binop->term) != nullptr;
        var_info_t* var;

        // If term is a function call save temporary value on stack
        if (is_function_call) {
            var = push_temp(t, left_register);
        }

        // Translate term
        right_register = binop->term->translate(t);

        // If term is a function call restore temporary value
        if (is_function_call) {
            left_register = pop_temp(t, var);
        }

    }
    
    tri_operand_instr(t, instr, left_register, left_register, right_register);

    return left_register;
}

int translate_binop_relational(translator_t* t, binop_expr_t* binop, const std::string& instr) {
    
    // Assume left associativity
    if (!binop->left_assoc) throw translation_error("Expression is right associative");
    
    int left_value = 0;
    bool left_success = binop->rest->evaluate(&left_value);

    int right_value = 0;
    bool right_success = binop->term->evaluate(&right_value);

    int left_register;
    int right_register;

    if (left_success) {

        // Means left was a constant, allocate a register and load the immediate value into it
        var_info_t* var_info;
        left_register = allocate_temp_imm(t, "__temp__", left_value, &var_info);

    } else {

        left_register = binop->rest->translate(t);

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

        bool is_function_call = dynamic_cast<call_term_t*>(binop->term) != nullptr;
        var_info_t* var;

        // If term is a function call save temporary value on stack
        if (is_function_call) {
            var = push_temp(t, left_register);
        }

        // Translate term
        right_register = binop->term->translate(t);

        // If term is a function call restore temporary value
        if (is_function_call) {
            left_register = pop_temp(t, var);
        }

        // Print cmp instruction
        cmp_instr(t, left_register, right_register);
    }

    std::string true_label = t->label_allocator.get_label_name();
    std::string end_label = t->label_allocator.get_label_name();
    
    // eg. breq L1
    branch_instr(t, instr, true_label);

    // addi r, NULL, 0
    addi_instr(t, left_register, NULL_REGISTER, 0);

    // jmp L2
    branch_instr(t, JMP_INSTR, end_label);

    // L1:
    print_label(t, true_label);

    // addi r, NULL, 1
    addi_instr(t, left_register, NULL_REGISTER, 1);

    // L2:
    print_label(t, end_label);

    return left_register;
}


std::string get_register_string(translator_t* t, int reg) {
    return (t->special_registers.count(reg)) ? t->special_registers[reg] : t->reg_alloc.get_register_string(reg);
}

void tri_operand_instr(translator_t* t, const std::string& instr, int rd, int ra, int rb) {

    std::string rd_str = get_register_string(t, rd);
    std::string ra_str = get_register_string(t, ra);
    std::string rb_str = get_register_string(t, rb);

    std::stringstream output;

    output  << instr << " " << rd_str << ", " << ra_str << ", " << rb_str;
    t->print_instruction_row(output.str(), true);
}

void tri_operand_imm_instr(translator_t* t, const std::string& instr, int rd, int ra, int imm) {

    std::string rd_str = get_register_string(t, rd);
    std::string ra_str = get_register_string(t, ra);

    std::stringstream output;

    output  << instr << " " << rd_str << ", " << ra_str << ", " << imm;
    t->print_instruction_row(output.str(), true);
}

void di_operand_instr(translator_t* t, const std::string& instr, int rd, int ra) {
    
    std::string ra_str = get_register_string(t, rd);
    std::string rb_str = get_register_string(t, ra);

    std::stringstream output;

    output << instr << " " << ra_str << ", " << rb_str;
    t->print_instruction_row(output.str(), true);
}

void di_operand_imm_instr(translator_t* t, const std::string& instr, int rd, int imm) {
    
    std::string ra_str = get_register_string(t, rd);

    std::stringstream output;

    output << instr << " " << ra_str << ", " << imm;
    t->print_instruction_row(output.str(), true);
}

void add_instr(translator_t* t, int rd, int ra, int rb) {

    tri_operand_instr(t, ADD_INSTR, rd, ra, rb);

}

void addi_instr(translator_t* t, int rd, int ra, int imm) {
    
    tri_operand_imm_instr(t, ADD_IMM_INSTR, rd, ra, imm);

}

void sub_instr(translator_t* t, int rd, int ra, int rb) {
    
    tri_operand_instr(t, SUB_INSTR, rd, ra, rb);

}

void subi_instr(translator_t* t, int rd, int ra, int imm) {
    
    tri_operand_imm_instr(t, SUB_IMM_INSTR, rd, ra, imm);

}

void mult_instr(translator_t* t, int rd, int ra, int rb) {
    
    tri_operand_instr(t, MULT_INSTR, rd, ra, rb);

}


void and_instr(translator_t* t, int rd, int ra, int rb) {
    
    tri_operand_instr(t, AND_INSTR, rd, ra, rb);

}

void or_instr(translator_t* t, int rd, int ra, int rb) {
    
    tri_operand_instr(t, OR_INSTR, rd, ra, rb);

}

void not_instr(translator_t* t, int rd, int ra) {

    di_operand_instr(t, NOT_INSTR, rd, ra);

}

void neg_instr(translator_t* t, int rd, int ra) {

    di_operand_instr(t, NEG_INSTR, rd, ra);

}

void xor_instr(translator_t* t, int rd, int ra, int rb) {

    tri_operand_instr(t, XOR_INSTR, rd, ra, rb);

}

void push_instr(translator_t* t, int rd, int size) {

    std::string rd_str = get_register_string(t, rd);

    std::stringstream output;

    t->symbol_table.get_current_scope()->push(size);

    output << PUSH_INSTR << "[" << size << "] " << rd_str;
    t->print_instruction_row(output.str(), true);

} 

void pop_instr(translator_t* t, int rd, int size) {

    std::string rd_str = get_register_string(t, rd);

    std::stringstream output;

    t->symbol_table.get_current_scope()->pop(size);

    output << POP_INSTR << "[" << size << "] " << rd_str;
    t->print_instruction_row(output.str(), true);
    
} 

void call_instr(translator_t* t, const std::string& function_identifier) {

    std::stringstream output;
    
    output << CALL_INSTR << " " << function_identifier;
    t->print_instruction_row(output.str(), true);
}

void ret_instr(translator_t* t) {

    std::stringstream output;

    output << RETURN_INSTR;
    t->print_instruction_row(output.str(), true, true);
}

void store_instr(translator_t* t, int rd, int ra, addr_info_t* offset, int size) {
    
    std::string rd_str = get_register_string(t, rd);
    std::string ra_str = get_register_string(t, ra);

    std::stringstream output;

    output << STORE_INSTR << "[" << size << "] " << rd_str << ", " << ra_str << ", " << offset->get_address_string();
    t->print_instruction_row(output.str(), true);
}

void load_instr(translator_t* t, int rd, int ra, addr_info_t* offset, int size) {
    
    std::string rd_str = get_register_string(t, rd);
    std::string ra_str = get_register_string(t, ra);

    std::stringstream output;

    output << LOAD_INSTR << "[" << size << "] " << rd_str << ", " << ra_str << ", " << offset->get_address_string();
    t->print_instruction_row(output.str(), true);
}

void movhi_instr(translator_t* t, int rd, int imm) {

    std::string rd_str = get_register_string(t, rd);

    std::stringstream output;

    output << MOVHI_INSTR << " " << rd_str << ", " << imm;
    t->print_instruction_row(output.str(), true);
}

void movlo_instr(translator_t* t, int rd, int imm) {

    std::string rd_str = get_register_string(t, rd);

    std::stringstream output;

    output << MOVLO_INSTR << " " << rd_str << ", " << imm;
    t->print_instruction_row(output.str(), true);
}

void move_instr(translator_t* t, int rd, int ra) {

    di_operand_instr(t, MOVE_INSTR, rd, ra);

}

void cmp_instr(translator_t* t, int ra, int rb) {
    
    di_operand_instr(t, CMP_INSTR, ra, rb);

}

void cmpi_instr(translator_t* t, int ra, int imm) {
    
    di_operand_imm_instr(t, CMP_IMM_INSTR, ra, imm);
    
}

void print_label(translator_t* t, const std::string& label) {

    std::stringstream output;

    output << label << ":";
    t->print_instruction_row(output.str(), false);
}

void branch_instr(translator_t* t, const std::string& instr, const std::string& label) {
    
    std::stringstream output;

    output << instr << " " << label;
    t->print_instruction_row(output.str(), true);

}
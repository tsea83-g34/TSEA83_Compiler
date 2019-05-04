
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

std::string get_register_string(translator_t* t, int reg) {
    return (t->special_registers.count(reg)) ? t->special_registers[reg] : t->reg_alloc.get_register_string(reg);
}

void add_instr(translator_t* t, int rd, int ra, int rb) {
    
    std::string rd_str = get_register_string(t, rd);
    std::string ra_str = get_register_string(t, ra);
    std::string rb_str = get_register_string(t, rb);

    std::stringstream output;

    output  << ADD_INSTR << " " << rd_str << ", " << ra_str << ", " << rb_str;
    t->print_instruction_row(output.str(), true);
}

void addi_instr(translator_t* t, int rd, int ra, int imm) {
    
    std::string rd_str = get_register_string(t, rd);
    std::string ra_str = get_register_string(t, ra);

    std::stringstream output;

    output  << ADD_IMM_INSTR << " " << rd_str << ", " << ra_str << ", " << imm;
    t->print_instruction_row(output.str(), true);

}

void sub_instr(translator_t* t, int rd, int ra, int rb) {
    
    std::string rd_str = get_register_string(t, rd);
    std::string ra_str = get_register_string(t, ra);
    std::string rb_str = get_register_string(t, rb);

    std::stringstream output;

    output  << SUB_INSTR << " " << rd_str << ", " << ra_str << ", " << rb_str;
    t->print_instruction_row(output.str(), true);

}

void subi_instr(translator_t* t, int rd, int ra, int imm) {
    
    std::string rd_str = get_register_string(t, rd);
    std::string ra_str = get_register_string(t, ra);

    std::stringstream output;

    output  << SUB_IMM_INSTR << " " << rd_str << ", " << ra_str << ", " << imm;
    t->print_instruction_row(output.str(), true);

}

void mult_instr(translator_t* t, int rd, int ra, int rb) {
    
    std::string rd_str = get_register_string(t, rd);
    std::string ra_str = get_register_string(t, ra);
    std::string rb_str = get_register_string(t, rb);

    std::stringstream output;

    output  << MULT_INSTR << " " << rd_str << ", " << ra_str << ", " << rb_str;
    t->print_instruction_row(output.str(), true);
}


void and_instr(translator_t* t, int rd, int ra, int rb) {
    
    std::string rd_str = get_register_string(t, rd);
    std::string ra_str = get_register_string(t, ra);
    std::string rb_str = get_register_string(t, rb);

    std::stringstream output;

    output  << AND_INSTR << " " << rd_str << ", " << ra_str << ", " << rb_str;
    t->print_instruction_row(output.str(), true);
}

void or_instr(translator_t* t, int rd, int ra, int rb) {
    
    std::string rd_str = get_register_string(t, rd);
    std::string ra_str = get_register_string(t, ra);
    std::string rb_str = get_register_string(t, rb);

    std::stringstream output;

    output  << OR_INSTR << " " << rd_str << ", " << ra_str << ", " << rb_str;
    t->print_instruction_row(output.str(), true);
}

void not_instr(translator_t* t, int rd, int ra) {

    std::string rd_str = get_register_string(t, rd);
    std::string ra_str = get_register_string(t, ra);

    std::stringstream output;

    output  << NOT_INSTR << " " << rd_str << ", " << ra_str;
    t->print_instruction_row(output.str(), true);

}

void neg_instr(translator_t* t, int rd, int ra) {

    std::string rd_str = get_register_string(t, rd);
    std::string ra_str = get_register_string(t, ra);

    std::stringstream output;

    output  << NEG_INSTR << " " << rd_str << ", " << ra_str;
    t->print_instruction_row(output.str(), true);

}

void xor_instr(translator_t* t, int rd, int ra, int rb) {

    std::string rd_str = get_register_string(t, rd);
    std::string ra_str = get_register_string(t, ra);
    std::string rb_str = get_register_string(t, rb);

    std::stringstream output;

    output << XOR_INSTR << " " << rd_str << ", " << ra_str << ", " << rb_str;
    t->print_instruction_row(output.str(), true);
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

    std::string rd_str = get_register_string(t, rd);
    std::string ra_str = get_register_string(t, ra);

    std::stringstream output;

    output << MOVE_INSTR << " " << rd_str << ", " << ra_str;
    t->print_instruction_row(output.str(), true);
}

void cmp_instr(translator_t* t, int ra, int rb) {
    
    std::string ra_str = get_register_string(t, ra);
    std::string rb_str = get_register_string(t, rb);

    std::stringstream output;

    output << CMP_INSTR << " " << ra_str << ", " << rb_str;
    t->print_instruction_row(output.str(), true);
}

void cmpi_instr(translator_t* t, int ra, int imm) {
    
    std::string ra_str = get_register_string(t, ra);

    std::stringstream output;

    output << CMP_IMM_INSTR << " " << ra_str << ", " << imm;
    t->print_instruction_row(output.str(), true);
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
#ifndef COM_HELPER_H
#define COM_HELPER_H

#include "translator.h"

#define POINTER_SIZE 2

void load_immediate(translator_t* t, int register_index, int value);

int allocate_temp_imm(translator_t* t, const std::string& name, int value, var_info_t** var);

var_info_t* give_ownership_temp(translator_t* t, const std::string& name, int reg);

var_info_t* push_temp(translator_t* t, int reg);

int pop_temp(translator_t* t, var_info_t* var);

// Instruction functions

void add_instr(translator_t* t,int rd, int ra, int rb);

void addi_instr(translator_t* t, int rd, int ra, int imm);

void sub_instr(translator_t* t, int rd, int ra, int rb);

void subi_instr(translator_t* t, int rd, int ra, int imm);

void neg_instr(translator_t* t, int rd, int ra);

void push_instr(translator_t* t, int rd, int size);

void pop_instr(translator_t* t, int rd, int size);

void call_instr(translator_t* t, const std::string& function_identifier);

void ret_instr(translator_t* t);

void store_instr(translator_t* t, int rd, int ra, addr_info_t* offset, int size);

void load_instr(translator_t* t, int rd, int ra, addr_info_t* offset, int size);

void movhi_instr(translator_t* t, int rd, int imm);

void movlo_instr(translator_t* t, int rd, int imm);

void cmp_instr(translator_t* t, int ra, int rb);

void cmpi_instr(translator_t* t, int ra, int imm);

void print_label(translator_t* t, const std::string& label);

void branch_instr(translator_t* t, const std::string& instr, const std::string& label);

std::string get_register_string(translator_t* t, int reg);

#endif
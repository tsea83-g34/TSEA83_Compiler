
#include "../include/translator.h"

#include <ostream>
#include <iostream>

void translation_error::throw_error(const std::string& error, const undoable_t* node) {

    if (node) {
        auto line = std::to_string(node->tokens.front()->line_number);
        auto column = std::to_string(node->tokens.front()->column_number);
        throw translation_error("\033[0;31m--- Translation Error\033[0m " + line + ":" + column + "\033[0;31m:\033[0m  " + error + "\n");
    } else {
        throw translation_error("\033[0;31m--- Translation Error:\033[0m " + error + "\n");
    }
}

translator_t::translator_t() {

    reg_alloc.set_parent(this);
    // print standard defines into file
    
    instr_cnt = 0L;
    last_was_ret = false;
    data_mode = false;
    
    set_data_mode(true);

    // Stack pointer
    print_instruction_row("const SP r15", false);
    special_registers.insert({15, "SP"});

    // Null register
    print_instruction_row("const NULL r14", false);
    special_registers.insert({14, "NULL"});

    // Base pointer
    print_instruction_row("const BP r13", false);
    special_registers.insert({13, "BP"});

    // Return register
    print_instruction_row("const RR r12", false);
    special_registers.insert({12, "RR"});

    // Print an empty row
    print_instruction_row("", false);

    set_data_mode(false);
}

void translator_t::set_data_mode(bool _mode) {
    data_mode = _mode;
}

bool translator_t::get_data_mode() {
    return data_mode;
}

void translator_t::print_instruction_row(const std::string& instr, bool tab, bool ret) {
    
    std::stringstream& local_output = (data_mode) ? defines_and_global_output : output; 

    last_was_ret = ret;
    if (tab) local_output << "\t";
    local_output << instr << "\n";
    instr_cnt++;
}

void translator_t::static_alloc(const std::string& name, int size, int value) {

    set_data_mode(true);
    
    std::string label = ".data " + name;

    std::string allocation;

    switch (size) {
        case 1:
            allocation += ".db ";
            break;
        case 2:
            allocation += ".dh ";
            break;
        case 4:
            allocation += ".dw ";
            break;
        default:
            break;
    }

    allocation += std::to_string(value);

    print_instruction_row(label, false);
    print_instruction_row(allocation, true);
    
    set_data_mode(false);
}

void translator_t::static_alloc_array(const std::string& name, int size, int length) {

    std::string label = ".data " + name;

    std::stringstream allocation;

    set_data_mode(true);

    switch (size) {
        case 1:
            allocation << ".db";
            break;
        case 2:
            allocation << ".dh";
            break;
        case 4:
            allocation << ".dw";
            break;
        default:
            break;
    }

    for (int i = 0; i < length; i++) allocation << " " << std::to_string(0); 

    print_instruction_row(label, false);
    print_instruction_row(allocation.str(), true);

    set_data_mode(false);
}

void translator_t::static_alloc_array_init(const std::string& name, int size, const std::vector<int>& values) {

    std::string label = ".data " + name;

    std::stringstream allocation;

    set_data_mode(true);

    switch (size) {
        case 1:
            allocation << ".db";
            break;
        case 2:
            allocation << ".dh";
            break;
        case 4:
            allocation << ".dw";
            break;
        default:
            break;
    }

    for (int val : values) allocation << " " << std::to_string(val);

    print_instruction_row(label, false);
    print_instruction_row(allocation.str(), true);

    set_data_mode(false); 

}

void translator_t::static_alloc_array_str(const std::string& name, const std::string& literal) {

    std::string label = ".data " + name;

    std::stringstream allocation;

    set_data_mode(true);

    allocation << ".ds " << literal;

    print_instruction_row(label, false);
    print_instruction_row(allocation.str(), true);

    set_data_mode(false);
}

void translator_t::print_to_file(std::ofstream& file) {

    set_data_mode(true);

    print_instruction_row("subi SP, SP, 1", true);
    print_instruction_row("call main", true);
    print_instruction_row("subi SP, SP, 2", true);
    print_instruction_row("__halt:", false);
    print_instruction_row("rjmp __halt", true);

    set_data_mode(false);

    file << defines_and_global_output.str() << "\n" << output.str();
}
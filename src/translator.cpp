
#include "../include/translator.h"

#include <ostream>

translator_t::translator_t() {

    reg_alloc.set_parent(this);
    // print standard defines into file
    
    // Stack pointer
    print_instruction_row("const SP r15", false);
    
    // Null register
    print_instruction_row("const NULL r14", false);
    
    // Base pointer
    print_instruction_row("const BP r13", false);
    
    // Return register
    print_instruction_row("const RR r12", false);

    // Print an empty row
    print_instruction_row("", false);
}

void translator_t::print_instruction_row(const std::string& instr, bool tab = true) {
    
    if (tab) output << "\t";
    output << instr << "\n";
    instr_cnt++;
}

void translator_t::static_alloc(std::string name, int size, int value) {
    
    std::string label = name;
    label += ":";

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
}

void translator_t::print_to_file(std::ofstream& file) {

    file << output.str();
}
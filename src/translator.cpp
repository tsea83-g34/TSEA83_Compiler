
#include "../include/translator.h"

#include <ostream>

void translator_t::print_instruction_row(const std::string& instr) {
    output << instr << std::endl;
    instr_cnt++;
}

void translator_t::static_alloc(std::string name, int size, int value) {
    
    std::string label = name;
    label += ":";

    std::string allocation = "\t";

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

    print_instruction_row(label);
    print_instruction_row(allocation);
}

void translator_t::print_to_file(std::ofstream& file) {

    file << output.str();
}
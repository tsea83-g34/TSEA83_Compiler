
#include "translator.h"

#include <ostream>

void translator_t::print_instruction_row(const std::string& instr) {
    output << instr << std::endl;
    instr_cnt++;
}

std::string static_alloc(int size, int value) {
    std::string result;

    switch (size) {
        case 1:
            result += ".db ";
            break;
        case 2:
            result += ".dh ";
            break;
        case 4:
            result += ".dw ";
            break;
        default:
            break;
    }

    result += std::to_string(value);
    return result;
}
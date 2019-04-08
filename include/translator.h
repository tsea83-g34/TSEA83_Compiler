#ifndef COM_TRANSLATOR_H
#define COM_TRANSLATOR_H

#include <sstream>

#include "symbol_table.h"
#include "register_allocation.h"
#include "type_table.h"

class translator_t {

    std::stringstream output;

public:
    symbol_table_t          symbol_table;
    register_allocator_t    reg_alloc;
    type_table_t            type_table;
    long instr_cnt;

    void print_instruction_row(const std::string& instr);
    std::string static_alloc(int size, int value);
};

#endif
#ifndef COM_TRANSLATOR_H
#define COM_TRANSLATOR_H

#include <sstream>
#include <string>
#include <fstream>
#include "symbol_table.h"
#include "type_table.h"
#include "register_allocation.h"

class translation_error : public std::exception {
private:
    std::string msg;
public:
    translation_error(std::string _msg) : msg(_msg) {}
    ~translation_error() {}

    const char* what() const noexcept { return msg.c_str(); }
};

class translator_t {

    std::stringstream output;

public:
    symbol_table_t          symbol_table;
    register_allocator_t    reg_alloc;
    type_table_t            type_table;
    long instr_cnt;

    translator_t();
    ~translator_t() = default;

    void print_to_file(std::ofstream& file);

    void print_instruction_row(const std::string& instr, bool tab);
    void static_alloc(std::string name, int size, int value);
};

#endif
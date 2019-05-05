#ifndef COM_TRANSLATOR_H
#define COM_TRANSLATOR_H

#include <sstream>
#include <string>
#include <fstream>
#include "symbol_table.h"
#include "type_table.h"
#include "register_allocation.h"

#define STACK_POINTER   15
#define NULL_REGISTER   14
#define BASE_POINTER    13
#define RETURN_REGISTER 12

class translation_error : public std::exception {
private:
    std::string msg;
public:
    translation_error(std::string _msg) : msg(_msg) {}
    ~translation_error() {}

    const char* what() const noexcept { return msg.c_str(); }
};

class translator_t {

    std::stringstream defines_and_global_output;
    std::stringstream output;

    bool data_mode;

public:

    std::unordered_map<int, std::string> special_registers;

    symbol_table_t          symbol_table;
    register_allocator_t    reg_alloc;
    type_table_t            type_table;
    scope_name_allocator_t  name_allocator;
    label_allocator_t       label_allocator;
    long instr_cnt;
    bool last_was_ret;

    translator_t();
    ~translator_t() = default;

    void print_to_file(std::ofstream& file);

    void set_data_mode(bool _mode);

    bool get_data_mode();

    void print_instruction_row(const std::string& instr, bool tab, bool ret = false);
    void static_alloc(std::string name, int size, int value);
};

#endif
#ifndef COM_REGISTER_ALLOC_H
#define COM_REGISTER_ALLOC_H

#include <vector>

#include "symbol_table.h"

class translator_t;

#define REGISTER_COUNT  16
#define RESERVE_COUNT   3

struct reg_t {
    int index;
    var_info_t* content;
    long last_changed;
    
    bool locked;    // The register is temporarily locked and cannot be modified
    bool temp;      // The register contains a temporary value and can be used as soon as possible
    bool reserved;  // The register can never be allocated

    reg_t();
};

class register_allocator_t {

    std::vector<reg_t*> registers;
    translator_t* parent;

    reg_t* get_register(int index);
    void free(reg_t* reg, bool sort = true);

public:

    register_allocator_t(); 
    ~register_allocator_t();

    void set_parent(translator_t* _parent);

    int allocate(var_info_t* var_to_alloc, bool temp);
    void free(int index);

    void store_context();

    static std::string get_register_string(int index);
};

#endif
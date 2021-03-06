#ifndef COM_REGISTER_ALLOC_H
#define COM_REGISTER_ALLOC_H

#include <vector>

#include "symbol_table.h"

class translator_t;

#define REGISTER_COUNT  16
#define RESERVE_COUNT   4

struct reg_t {
    int index;
    var_info_t* content;
    long last_changed;
    
    bool locked;        // The register is temporarily locked and cannot be modified
    bool changed;       // Keeps track of if the register value has changed since loading the variable
    bool temp;          // The register contains a temporary value
    bool reserved;      // The register can never be allocated

    reg_t();
};

class register_allocator_t {

    std::vector<reg_t*> registers;
    translator_t* parent;

    reg_t* get_register(int index);
    void free(reg_t* reg, bool store, bool sort = true);

public:

    register_allocator_t(); 
    ~register_allocator_t();

    void set_parent(translator_t* _parent);

    int allocate(var_info_t* var_to_alloc, bool load_variable, bool temp);

    bool already_allocated(var_info_t* var);

    // Frees the given register and returns the content descriptor
    var_info_t* free(int index);
    
    // If a given variable is stored in a register, frees it and potentially stores
    void free(var_info_t* var, bool store);

    void store_context();
    void free_scope(scope_t* scope_to_free, bool store_globals = false);

    void touch(int register_index, bool has_changed);
    bool is_temporary(int register_index);

    // Changes the content of the register, without loading or storing anything
    // used when a register already has the desired value but the value changes variable
    // ie. a temporary expression is assigned to a variable. Returns the old variable
    var_info_t* give_ownership(int register_index, var_info_t* new_owner);

    static std::string get_register_string(int index);
};

#endif
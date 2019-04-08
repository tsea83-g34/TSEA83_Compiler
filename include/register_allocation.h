#ifndef COM_REGISTER_ALLOC_H
#define COM_REGISTER_ALLOC_H

#include <vector>

#include "translator.h"
#include "symbol_table.h"

#define REGISTER_COUNT  16
#define RESERVE_COUNT   1

struct register_t {
    int index;
    var_info_t* content;
    long last_changed;
    
    bool locked;    // The register is temporarily locked and cannot be modified
    bool temp;      // The register contains a temporary value and can be used as soon as possible
    bool reserved;  // The register can never be allocated

    register_t();
};

class register_allocator_t {

    std::vector<register_t*> registers;
    const translator_t* parent;

    register_t* get_register(int index);

public:

    register_allocator_t(); 
    ~register_allocator_t();

    int allocate(var_info_t* var_to_alloc, bool lock);
    void free(int index);

    static std::string get_register_string(int index);
};

#endif

#include "../include/register_allocation.h"

#include "../include/translator.h"
#include "../include/instructions.h"
#include "../include/helper_functions.h"


#include <algorithm>
#include <functional>
#include <sstream>
#include <iostream>

reg_t::reg_t() {
    index        = 0;
    content      = nullptr;
    last_changed = 0L;

    locked      = false;
    temp        = false;
    changed     = false;
    reserved    = false;

}

struct reg_ptr_cmp {
    bool operator()(const reg_t* left, const reg_t* right) {
        return left->last_changed > right->last_changed;
    }
};

register_allocator_t::register_allocator_t() {
    //registers = std::vector<reg_t*>(REGISTER_COUNT);

    for (int i = 0; i < REGISTER_COUNT; i++) {
        
        reg_t* reg = new reg_t();
        reg->index = i;
        registers.push_back(reg);

        if (i >= REGISTER_COUNT - RESERVE_COUNT) {
            reg->reserved = true;
            reg->last_changed = std::numeric_limits<long>::max();
        }
    }

    //std::make_heap(registers.begin(), registers.end(), reg_ptr_cmp());
}

register_allocator_t::~register_allocator_t() {
    
    for (auto* reg : registers) {
        delete reg;
    }
    registers.clear();
}

void register_allocator_t::set_parent(translator_t* _parent) {
    parent = _parent;
}

reg_t* register_allocator_t::get_register(int index) {

    for (int i = 0; i < registers.size(); i++) {
        if (registers[i]->index == index) return registers[i];
    }
    return nullptr;
}

void register_allocator_t::free(reg_t* reg, bool store, bool sort) {


    // If the register has no content, there is nothing to free
    if (reg->content == nullptr) return;

    var_info_t* old_data = reg->content;
    int size = (old_data->is_pointer) ? POINTER_SIZE : parent->type_table.at(old_data->type)->size;

    if (store && reg->changed) {
        
        int base_or_null;

        if (dynamic_cast<global_addr_info_t*>(reg->content->address) != nullptr) {
            // If variable is global is zero
            base_or_null = NULL_REGISTER;
        } else {
            // If variable is local address is base pointer
            base_or_null = BASE_POINTER;
        }

        // Store variable
        store_instr(parent, base_or_null, reg->index, old_data->address, size);
    }
    
    // Update register
    reg->temp = false;
    reg->content = nullptr;
    reg->last_changed = 0;
    reg->changed = false;
    
    // re-heapify the vector
    //if (sort) std::make_heap(registers.begin(), registers.end(), std::greater<reg_t*>());
}

int register_allocator_t::allocate(var_info_t* var_to_alloc, bool load_variable, bool temp) {


    if (var_to_alloc == nullptr) {
        translation_error::throw_error("Allocating register to non-existent variable", nullptr);
    }

    for (int i = 0; i < registers.size(); i++) {
        reg_t* reg = registers[i];

        if (reg->content == nullptr) continue;
        
        if (reg->content == var_to_alloc) {
            // If the allocation is temporary, make it available instantly
            reg->last_changed = (temp) ? 0 : parent->instr_cnt;
            return reg->index;
        }
    }

    long min = std::numeric_limits<long>().max();
    reg_t* front = nullptr; 

    for (auto* reg : registers) {
        if (reg->last_changed < min) {
            front = reg;
            min = reg->last_changed;
        }
    }
    
    // Free the register, store the variable and dont sort the heap
    // if the variable contained is temporary, don't store
    if (front->content != nullptr) free(front, !(front->temp), false);

    // ----- Variable loading -----
    
    if (load_variable) {
        // Load the variable into the register
        
        int base_or_null;
        if (dynamic_cast<global_addr_info_t*>(var_to_alloc->address) != nullptr) {
            // If the variable is global use null register
            base_or_null = NULL_REGISTER;

        } else {
            // If the variable is local use base pointer
            base_or_null = BASE_POINTER;
        }

        int size = (var_to_alloc->is_pointer) ? POINTER_SIZE : parent->type_table.at(var_to_alloc->type)->size;
        
        // Print instruction
        load_instr(parent, front->index, base_or_null, var_to_alloc->address, size);
    }

    // ----------------------------

    // Update the register
    front->temp = temp;
    front->content = var_to_alloc;
    front->last_changed = (temp) ? 0 : parent->instr_cnt;
    front->changed = false;

    return front->index;
}

var_info_t* register_allocator_t::free(int index) {

    // Get register
    reg_t* reg = get_register(index);
    var_info_t* old_content = reg->content;
    
    // Free the register, store the variable and sort the heap
    free(reg, true, true);
    return old_content;
}

void register_allocator_t::free(var_info_t* var, bool store) {

    for (auto* reg : registers) {

        if (reg->content == var) {
            free(reg, store, false);
            return;
        }
        
    }

}

void register_allocator_t::store_context() {

    for (reg_t* reg : registers) {
        if (reg->content == nullptr) continue;

        if (!parent->symbol_table.is_scope_reachable(reg->content->scope)) continue;

        // Free the register, store the variable and dont sort the heap
        free(reg, true, false);

    }

    // re-heapify the vector
    //std::make_heap(registers.begin(), registers.end(), reg_ptr_cmp());

}

void register_allocator_t::free_scope(scope_t* scope_to_free, bool store_globals) {
    
    scope_t* global_scope = parent->symbol_table.get_global_scope();
    
    for (reg_t* reg : registers) {

        if (reg->content == nullptr) continue;
 
        bool globals = store_globals != (reg->content->scope == global_scope);
        if (reg->content->scope != scope_to_free && globals) continue;

        bool store = reg->content->scope != scope_to_free;

        // Free the register and store it if it is a global or belongs to an outer scope
        free(reg, store, false);
    }

    // re-heapify the vector
    //std::make_heap(registers.begin(), registers.end(), reg_ptr_cmp());

}

void register_allocator_t::touch(int register_index, bool has_changed) {

    // Acquire register
    reg_t* reg = get_register(register_index);

    // Update timer
    reg->last_changed = parent->instr_cnt;
    reg->changed      = has_changed;

    // Re-heapify the vector
    //std::make_heap(registers.begin(), registers.end(), reg_ptr_cmp());
} 

var_info_t* register_allocator_t::give_ownership(int register_index, var_info_t* new_owner) {


    std::stringstream output;



    reg_t* reg = get_register(register_index);

    var_info_t* old_content = reg->content;

    // If variable is not temporary, store it
    free(reg, !reg->temp, false);

    reg->content = new_owner;
    reg->last_changed = parent->instr_cnt;
    reg->temp = false;  
    reg->changed = false;

    //std::make_heap(registers.begin(), registers.end(), reg_ptr_cmp());

    return old_content;
}

bool register_allocator_t::is_temporary(int register_index) {
    reg_t* reg = get_register(register_index);
    if (reg == nullptr) return false;
    if (reg->content == nullptr) return false;
    return reg->content->is_temp;
}

std::string register_allocator_t::get_register_string(int index) {
    return "r" + std::to_string(index);
}
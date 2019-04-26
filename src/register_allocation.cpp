
#include "../include/register_allocation.h"

#include "../include/translator.h"
#include "../include/instructions.h"


#include <algorithm>
#include <functional>
#include <sstream>
#include <iostream>

reg_t::reg_t() {
    index        = 0;
    content      = nullptr;
    last_changed = 0;

    locked      = false;
    temp        = false;
    reserved    = false;

}

namespace std {
    template<>
    struct greater<reg_t*> {
        bool operator()(reg_t* left, reg_t* right) {
            return left->last_changed > right->last_changed;
        }
    };
}

register_allocator_t::register_allocator_t() {

    registers = std::vector<reg_t*>(REGISTER_COUNT);

    for (int i = 0; i < REGISTER_COUNT; i++) {
        reg_t* reg = new reg_t();
        reg->index = i;
        registers[i] = reg;
        if (i >= REGISTER_COUNT - RESERVE_COUNT) {
            reg->reserved = true;
            reg->last_changed = std::numeric_limits<long>::max();
        }
    }

    std::make_heap(registers.begin(), registers.end(), std::greater<reg_t*>());
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
    int size = parent->type_table.at(old_data->type)->size;

    if (store) {
        
        // Store variable
        std::stringstream output;
        output << STORE_INSTR << "[" << size << "]"; 
        
        if (dynamic_cast<global_addr_info_t*>(reg->content->address) != nullptr) {
            // If variable is global is zero
            output << " NULL, ";
        } else {
            // If variable is local address is base pointer
            output << " BP, ";
        }
        
        output << get_register_string(reg->index) << ", " << old_data->address->get_address_string(); 
        parent->print_instruction_row(output.str(), true);    
    }
    
    // Update register
    reg->content = nullptr;
    reg->last_changed = 0;
    
    // re-heapify the vector
    if (sort) std::make_heap(registers.begin(), registers.end(), std::greater<reg_t*>());
}

int register_allocator_t::allocate(var_info_t* var_to_alloc, bool load_variable, bool temp = false) {

    if (var_to_alloc == nullptr) return -1;

    for (int i = 0; i < registers.size(); i++) {
        reg_t* reg = registers[i];
        
        if (reg->content == var_to_alloc) {
            // If the allocation is temporary, make it available instantly
            reg->last_changed = (temp) ? 0 : parent->instr_cnt;
            return reg->index;
        }
    }

    std::cout << "Allocating register..." << std::endl;

    // Pop the least recently changed register from the heap to modify
    reg_t* front = registers.front();
    std::pop_heap(registers.begin(), registers.end(), std::greater<reg_t*>());

    std::cout << "Allocated register " << front->index << ", last changed: " << front->last_changed << std::endl;
    
    // Free the register, store the variable and dont sort the heap
    free(front, true, false);

    // ----- Variable loading -----
    
    if (load_variable) {
        // Load the variable into the register
        std::stringstream output;
        output << LOAD_INSTR << "[" << parent->type_table.at(var_to_alloc->type)->size << "] ";
        output << get_register_string(front->index);

        if (dynamic_cast<global_addr_info_t*>(var_to_alloc->address) != nullptr) {
            // If the variable is global use null register
            output << ", NULL, ";

        } else {
            // If the variable is local use base pointer
            output << ", BP, "; 
        }

        // Use variable adress as offset
        output << var_to_alloc->address->get_address_string();
        
        // Print instruction
        parent->print_instruction_row(output.str(), true);
    }

    // ----------------------------

    // Update the register
    front->content = var_to_alloc;
    front->last_changed = (temp) ? 0 : parent->instr_cnt;

    // Push the updated register back to the heap
    std::push_heap(registers.begin(), registers.end(), std::greater<reg_t*>());
    
    return front->index;
}

void register_allocator_t::free(int index) {

    // Get register
    reg_t* reg = get_register(index);
    
    // Free the register, store the variable and sort the heap
    free(reg, true, true);
}

void register_allocator_t::store_context() {

    for (reg_t* reg : registers) {

        if (!parent->symbol_table.is_scope_reachable(reg->content->scope)) continue;

        // Free the register, store the variable and dont sort the heap
        free(reg, true, false);

    }

    // re-heapify the vector
    std::make_heap(registers.begin(), registers.end(), std::greater<reg_t*>());

}

void register_allocator_t::free_scope(scope_t* scope_to_free) {

    for (reg_t* reg : registers) {

        if (reg->content == nullptr) continue;

        if (reg->content->scope != scope_to_free) continue;

        // Free the register without storing the variable or sorting the heap
        free(reg, false, false);
    }

    // re-heapify the vector
    std::make_heap(registers.begin(), registers.end(), std::greater<reg_t*>());

}

std::string register_allocator_t::get_register_string(int index) {
    return "r" + std::to_string(index);
}
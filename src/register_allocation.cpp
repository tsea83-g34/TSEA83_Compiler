
#include "../include/register_allocation.h"

#include "../include/translator.h"

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

void register_allocator_t::free(reg_t* reg, bool sort) {

    std::stringstream output;

    // If the register has no content, there is nothing to free
    if (reg->content == nullptr) return;


    var_info_t* old_data = reg->content;
    int size = parent->type_table.at(old_data->type)->size;

    // Store variable
    output << "store[" << size << "] BP, " << get_register_string(reg->index) << ", " << old_data->address->get_address_string(); 
    parent->print_instruction_row(output.str(), true);
    
    // Update register
    reg->content = nullptr;
    reg->last_changed = 0;
    
    // re-heapify the vector
    if (sort) std::make_heap(registers.begin(), registers.end(), std::greater<reg_t*>());
}

int register_allocator_t::allocate(var_info_t* var_to_alloc, bool temp = false) {

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
    
    // Deallocate the register
    free(front, false);

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
    
    free(reg, true);
}

void register_allocator_t::store_context() {

    for (reg_t* reg : registers) {

        if (!parent->symbol_table.is_scope_reachable(reg->content->scope)) continue;

        free(reg, false);

    }

    // re-heapify the vector
    std::make_heap(registers.begin(), registers.end(), std::greater<reg_t*>());

}

std::string register_allocator_t::get_register_string(int index) {
    return "r" + std::to_string(index);
}
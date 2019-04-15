
#include "../include/register_allocation.h"

#include "../include/translator.h"

#include <algorithm>
#include <functional>

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
        if (i < RESERVE_COUNT) {
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

reg_t* register_allocator_t::get_register(int index) {

    for (int i = 0; i < registers.size(); i++) {
        if (registers[i]->index == index) return registers[i];
    }
    return nullptr;
}

int register_allocator_t::allocate(var_info_t* var_to_alloc, bool lock = false) {

    if (var_to_alloc == nullptr) return -1;

    for (int i = 0; i < registers.size(); i++) {
        reg_t* reg = registers[i];
        
        if (reg->content == var_to_alloc) {
            reg->last_changed = parent->instr_cnt;
            return reg->index;
        }
    }

    // Pop the least recently changed register from the heap to modify
    reg_t* front = registers.front();
    std::pop_heap(registers.begin(), registers.end(), std::greater<reg_t*>());

    // Update the register
    front->content = var_to_alloc;
    front->last_changed = parent->instr_cnt;

    // Push the updated register back to the heap
    std::push_heap(registers.begin(), registers.end(), std::greater<reg_t*>());
    
    // TODO print instructions
}

void register_allocator_t::free(int index) {

    // Get register
    reg_t* reg = get_register(index);
    
    // Update register
    reg->content = nullptr;
    reg->last_changed = 0;
    
    // re-heapify the vector
    std::make_heap(registers.begin(), registers.end(), std::greater<reg_t*>());
}

std::string register_allocator_t::get_register_string(int index) {
    return "r" + std::to_string(index);
}
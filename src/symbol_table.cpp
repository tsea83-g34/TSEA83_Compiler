
#include "../include/symbol_table.h"
#include "../include/translator.h"

#include <iostream>


std::string global_addr_info_t::get_address_string() {
    return label;
}

std::string local_addr_info_t::get_address_string() {
    
    return "BP+" + std::to_string(base_offset);
}


scope_name_allocator_t::scope_name_allocator_t() {
    name_counter = std::unordered_map<std::string, int>();
}

std::string scope_name_allocator_t::get_name(const std::string& id) {

    if (name_counter.count(id)) {
        return id + ":" + std::to_string(name_counter[id]++);
    } else {
        name_counter.insert({id, 1});
        return id + ":0";
    }
}

func_info_t::func_info_t(func_decl_t* decl, translator_t* t) {
    
    identifier = decl->id;
    return_type = decl->type;
    params_size = 0;

    // If the function has no parameters, finish
    if (decl->param_list == nullptr) return;

    // Loop over parameters and add them to the param_info vector
    param_decls_t* current = decl->param_list;

    // Starts at 2 to accomodate for return address pointer
    int current_base_offset = 2;

    while (current != nullptr) {
        
        var_info_t param_info;
        param_info.name = current->first->id;
        param_info.id = current->first->id;
        param_info.type = current->first->type;

        int current_size = t->type_table.at(param_info.type)->size;

        // If the current base offset is mis-aligned in relation to the size of this parameter
        // adjust it 
        if (current_base_offset % current_size != 0) {
            current_base_offset += (-current_base_offset) % current_size;
        }

        // Create address structure for parameter
        local_addr_info_t* addr = new local_addr_info_t(current_base_offset);
        param_info.address = addr;
        
        // Update base offset with the new variable
        current_base_offset += current_size;

        param_vector.push_back(param_info);        
        current = current->rest;
    }

    // Align stack to 4
    if (current_base_offset % 4 != 0) {
            current_base_offset += (-current_base_offset) % 4;
    }

    // params_size equals the current base offset minus the return pointer
    params_size = current_base_offset - 2;
}

scope_t::scope_t() : inherit_scope(false), base_offset(0) {
    data = std::unordered_map<std::string, var_info_t*>();
}

scope_t::~scope_t() {
    
    std::cout << "Destroying scope... size: " << data.size() << std::endl;
    for (std::pair<std::string, var_info_t*> kv_pair : data) {
        std::cout << kv_pair.first << "   " << kv_pair.second << std::endl;
        delete kv_pair.second;
    }
    data.clear();
}

scope_t::scope_t(bool _inherit_scope, int _base_offset) : inherit_scope(_inherit_scope) {
    
    data = std::unordered_map<std::string, var_info_t*>();
    base_offset = _base_offset;
}

int scope_t::size() {
    return total_size;
}

int scope_t::get_base_offset() {
    return base_offset;
}

int scope_t::get_end_offset() {
    return base_offset + total_size;
}

int scope_t::align(int size_to_align_to) {
    
    if (size_to_align_to == 1) return 0;
    if (total_size % size_to_align_to == 0) return 0;

    int alignment = size_to_align_to - (total_size % size_to_align_to);
    total_size += alignment;

    return alignment;
}

var_info_t* scope_t::at(const std::string& key) {
    if (data.count(key)) {
        return data[key];
    } else {
        return nullptr;
    }
}

var_info_t* scope_t::operator[](const std::string& key) {
    return data[key];
}

void scope_t::add(const std::string& name, int size, var_info_t* varinfo) {
    total_size += size;
    data.insert({name, varinfo});
}


symbol_table_t::symbol_table_t() {
    scope_stack = std::deque<scope_t*>();
    name_alloc = scope_name_allocator_t(); 

    // Create global scope
    push_scope(false);
}

symbol_table_t::~symbol_table_t() {

    while (!scope_stack.empty()) {
        pop_scope();
    }
}

var_info_t* symbol_table_t::get_var(const std::string& key) {

    var_info_t* result = nullptr;

    // Loop goes until second scope because global scope is handled separately
    for (int i = scope_stack.size(); i >= 1; i++) {
        result = scope_stack[i]->at(key);
        if (result != nullptr) return result;

        if (!scope_stack[i]->inherit_scope) break; // If outer scope is not inherited, break
    }

    // If symbol not found yet, look at the global scope
    result = scope_stack.front()->at(key);

    return result;
}

var_info_t* symbol_table_t::add_var(const std::string& name, const int type, const int size, addr_info_t* addr) {

    var_info_t* varinfo = new var_info_t();
    varinfo->name = name;
    varinfo->type = type;
    varinfo->address = addr;

    if (is_global_scope()) {
        varinfo->id = name;
    } else {
        varinfo->id = name_alloc.get_name(name);
    }

    scope_stack.back()->add(name, size, varinfo);
    return varinfo;
}

func_info_t* symbol_table_t::get_func(const std::string& name) {
    if (function_table.count(name)) {
        return function_table[name];
    } else {
        return nullptr;
    }
}

std::string symbol_table_t::add_func(const std::string& name, func_info_t* f) {
    // TODO: Check if a function with that name already exists
    function_table.insert({name, f});
}

bool symbol_table_t::is_global_scope() {
    return scope_stack.size() == 1;
}

scope_t* symbol_table_t::get_current_scope() {
    return scope_stack.back();
}

void symbol_table_t::push_scope(bool inherit_scope) {
    
    scope_t* new_scope;
    
    if (inherit_scope) {
        scope_t* prev = scope_stack.back();
        new_scope = new scope_t(true, prev->get_end_offset());
    } else {
        new_scope = new scope_t();
    }

    scope_stack.push_back(new_scope);
}

void symbol_table_t::pop_scope() {
    
    scope_t* top = scope_stack.back();
    scope_stack.pop_back();
    delete top;
}
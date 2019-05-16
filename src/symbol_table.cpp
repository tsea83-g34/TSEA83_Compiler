
#include "../include/symbol_table.h"
#include "../include/translator.h"
#include "../include/helper_functions.h"

#include <iostream>
#include <utility>


std::string global_addr_info_t::get_address_string() {
    return label;
}

std::string local_addr_info_t::get_address_string() {
    
    return std::to_string(base_offset);
}

scope_name_allocator_t::scope_name_allocator_t() {
    name_counter = std::unordered_map<std::string, int>();
}

label_allocator_t::label_allocator_t() {
    counter = 0;
}

std::string label_allocator_t::get_label_name() {
    std::string result = std::string("L") + std::to_string(counter);
    counter++; 
    return result;
}

std::string scope_name_allocator_t::get_name(const std::string& id) {

    if (name_counter.count(id)) {
        return id + ":" + std::to_string(name_counter[id]++);
    } else {
        name_counter.insert({id, 1});
        return id + ":0";
    }
}

bool var_info_t::operator==(const var_info_t& other) {

    return  name    == other.name &&
            type    == other.type &&
            address->get_address_string() == other.address->get_address_string() &&
            is_pointer == other.is_pointer;
}

func_info_t::func_info_t(const func_decl_t* decl, translator_t* t) {

    
    identifier = decl->id;
    return_type = decl->type;
    params_size = 0;
    total_stack_size = 0;
    defined = false;

    // If the function has no parameters, finish
    if (decl->param_list == nullptr) return;

    // Loop over parameters and add them to the param_info vector
    param_decls_t* current = decl->param_list;

    // Loop over params and build param type vector
    std::vector<int> param_types;
    int param_count = 0;
    while (current != nullptr) {
        if (current->first->is_pointer) {
            param_types.push_back(0);
        } else {
            param_types.push_back(current->first->type);
        }
        param_count++;
        current = current->rest;
    }
    
    // Loop backwards over param_types and calculate stack alignment for passing parameters
    int total = 0;
    while (!param_types.empty()) {
        
        int current_type = param_types.back();
        int current_size = t->type_table.at(current_type)->size;
        //if (current_size == 1) current_size = 2;

        int alignment = 0;
        if (total % current_size) {
            alignment = current_size - (total % current_size);
            total += alignment;
            alignment_vector.push_back(alignment_info_t(param_types.size() - 1, alignment));
        }

        total += current_size;
        param_types.pop_back();
    }

    total_stack_size = total;

    // Loop over parameters and add them to the param_info vector
    current = decl->param_list;

    // If the parameters on stack are 4 aligned, two bytes will be pushed for alignment
    // Therefore offset start will be 4, otherwise 2
    int current_base_offset = 2 + (6 - total_stack_size % 4) % 4;
    int param_index = 0;

    while (current != nullptr) {
        
        var_info_t param_info;
        param_info.name = current->first->id;
        param_info.id = current->first->id;
        param_info.type = current->first->type;
        param_info.is_pointer = current->first->is_pointer;

        int current_size = (param_info.is_pointer) ? POINTER_SIZE : t->type_table.at(param_info.type)->size;

        //if (current_size == 1) current_size = 2;


        // If the current base offset is mis-aligned in relation to the size of this parameter
        // adjust it 
        /*if (current_base_offset % current_size != 0) {
            int param_alignment = current_size - current_base_offset % current_size;
            current_base_offset += param_alignment;
        }*/

        // Create address structure for parameter
        local_addr_info_t* addr = new local_addr_info_t(current_base_offset);
        param_info.address = addr;

        // Update base offset with the new variable
        current_base_offset += current_size;

        int alignment = get_alignment(param_index);

        if (alignment) {
            current_base_offset += alignment;
        }

        param_vector.push_back(param_info);        
        current = current->rest;
        param_index++;
    }
}

bool func_info_t::operator==(const func_info_t& other) {

    if (params_size != other.params_size) return false;
    if (param_vector.size() != other.param_vector.size()) return false;

    bool params_equal = true;

    for (int i = 0; i < params_size; i++) {
        params_equal &= param_vector[i] == other.param_vector[i];
    }

    return  identifier      == other.identifier &&
            return_type     == other.return_type && params_equal;
}

bool func_info_t::operator!=(const func_info_t& other) {
    return !(*this == other);
}

int func_info_t::get_alignment(int param_index) {
    for (auto& al : alignment_vector) {
        if (al.param_index == param_index) return al.alignment;
    }
    return 0;
}

scope_t::scope_t() : total_size(0), base_offset(0), inherit_scope(false) {
    data = std::unordered_map<std::string, var_info_t*>();
}

scope_t::scope_t(bool _inherit_scope, int _base_offset) : total_size(0), base_offset(_base_offset), inherit_scope(_inherit_scope) {
    data = std::unordered_map<std::string, var_info_t*>();
}

scope_t::~scope_t() {
    
    for (std::pair<std::string, var_info_t*> kv_pair : data) {
        delete kv_pair.second;
    }
    data.clear();
}

void scope_t::push(int size) {
    total_size += size;
}

void scope_t::pop(int size) {
    total_size -= size;
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

void scope_t::remove(const std::string& name) {
    data.erase(name);
}


symbol_table_t::symbol_table_t() {
    name_alloc = scope_name_allocator_t(); 

    // Create global scope
    push_scope(false);
}

symbol_table_t::~symbol_table_t() {

    while (!scope_stack.empty()) {
        pop_scope();
    }

    for (auto kv_pair : function_table) {
        delete kv_pair.second;
    }
    function_table.clear();
}

var_info_t* symbol_table_t::get_var(const std::string& key) {

    var_info_t* result = nullptr;

    // Loop goes until second scope because global scope is handled separately
    for (int i = scope_stack.size() - 1; i >= 1; i--) {
        result = scope_stack[i]->at(key);
        if (result != nullptr) return result;

        if (!scope_stack[i]->inherit_scope) break; // If outer scope is not inherited, break
    }

    // If symbol not found yet, look at the global scope
    result = scope_stack.front()->at(key);

    if (!result) translation_error::throw_error("Variable " + key + " does not exist in the symbol table", nullptr);
    return result;
}

var_info_t* symbol_table_t::add_var(const std::string& name, const int type, const int size, addr_info_t* addr) {

    var_info_t* varinfo = new var_info_t();
    varinfo->name = name;
    varinfo->type = type;
    varinfo->scope = get_current_scope();
    varinfo->address = addr;
    varinfo->is_pointer = false;

    if (is_global_scope()) {
        varinfo->id = name;
    } else {
        varinfo->id = name_alloc.get_name(name);
    }

    get_current_scope()->add(name, size, varinfo);
    return varinfo;
}

func_info_t* symbol_table_t::get_func(const std::string& name) {
    if (function_table.count(name)) {
        return function_table[name];
    } else {
        return nullptr;
    }
}

void symbol_table_t::add_func(const std::string& name, func_info_t* f) {

    // TODO: Check if a function with that name already exists
    function_table.insert(std::make_pair(name, f));
}

void symbol_table_t::remove_func(const std::string& name) {
    
    func_info_t* info = get_func(name);

    if (info == nullptr) return;

    function_table.erase(name);

    delete info;
}

bool symbol_table_t::is_scope_reachable(scope_t* scope) {

    if (scope == get_global_scope()) return true;

    for (int i = scope_stack.size() - 1; i >= 1; i--) {
        
        if (scope_stack[i] == scope) return true;

        if (!scope_stack[i]->inherit_scope) break; // If outer scope is not inherited, break
    }
    return false;
}

bool symbol_table_t::is_global_scope() {
    return scope_stack.size() == 1;
}

scope_t* symbol_table_t::get_current_scope() {
    return scope_stack.back();
}

scope_t* symbol_table_t::get_global_scope() {
    return scope_stack.front();
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
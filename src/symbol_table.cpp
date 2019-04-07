
#include "symbol_table.h"

std::string scope_name_allocator_t::get_name(const std::string& id) {

    if (name_counter.count(id)) {
        return id + ":" + std::to_string(name_counter[id]++);
    } else {
        name_counter.insert({id, 1});
        return id + ":0";
    }
}

scope_t::scope_t() : inherit_scope(false), base_offset(0) {
    data = std::unordered_map<std::string, var_info_t>();
}

scope_t::scope_t(bool _inherit_scope, int _base_offset) : inherit_scope(_inherit_scope) {
    
    data = std::unordered_map<std::string, var_info_t>();
    base_offset = _base_offset;
}

int scope_t::size() {
    return total_size;
}

int scope_t::get_end_offset() {
    return base_offset + total_size;
}

var_info_t* scope_t::at(const std::string& key) {
    if (data.count(key)) {
        return &data[key];
    } else {
        return nullptr;
    }
}

var_info_t* scope_t::operator[](const std::string& key) {
    return &data[key];
}

void scope_t::add(const std::string& name, const var_info_t& varinfo) {
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
        delete scope_stack.back();
        scope_stack.pop_back();
    }
}

var_info_t* symbol_table_t::get(const std::string& key) {

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

var_info_t* symbol_table_t::operator[](const std::string& key) {
    return this->get(key);
}

std::string symbol_table_t::add(const std::string& name, const int type) {
    std::string id = name_alloc.get_name(name);

    var_info_t varinfo = (var_info_t){name, id, type, (addr_info_t){}};
    scope_stack.back()->add(name, varinfo);
    return id;
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
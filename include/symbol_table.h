#ifndef COM_SCOPE_H
#define COM_SCOPE_H

#include <unordered_map>
#include <string>
#include <deque>

struct addr_info_t {
    int base_offset;
};

class scope_name_allocator_t {
    std::unordered_map<std::string, int> name_counter;

public:
    scope_name_allocator_t();
    std::string get_name(const std::string& id);
};

struct var_info_t {
    std::string name;
    std::string id;
    int type;
    addr_info_t address; // Relative address to the base pointer ?
};

class scope_t {
    std::unordered_map<std::string, var_info_t> data;
    
    int total_size;
    int base_offset;

public:
    
    const bool inherit_scope;
    
    scope_t();
    scope_t(bool _inherit_scope, int _base_offset);
    
    int size();
    int get_end_offset();
    
    var_info_t* at(const std::string& key);
    var_info_t* operator[](const std::string& key);

    void add(const std::string& name, const var_info_t& varinfo);
};

class symbol_table_t {
    std::deque<scope_t*> scope_stack;
    scope_name_allocator_t name_alloc;

public:
    symbol_table_t();
    ~symbol_table_t();

    var_info_t* get(const std::string& key);
    var_info_t* operator[](const std::string& key);

    std::string add(const std::string& name, const int type);
    
    bool is_global_scope();

    void push_scope(bool inherit_scope);
    void pop_scope();
};

#endif
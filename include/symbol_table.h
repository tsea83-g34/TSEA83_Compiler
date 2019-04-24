#ifndef COM_SCOPE_H
#define COM_SCOPE_H

#include <unordered_map>
#include <string>
#include <deque>

struct addr_info_t {
    
    virtual std::string get_address_string() = 0;
};

struct global_addr_info_t : addr_info_t {
    
    std::string label;
    
    global_addr_info_t(const std::string& _label) : label(_label) { }
    std::string get_address_string() override;
};

struct local_addr_info_t : addr_info_t {

    int base_offset;
    
    local_addr_info_t(int _base_offset) : base_offset(_base_offset) { }
    std::string get_address_string() override;
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
    addr_info_t* address; // Relative address to the base pointer ?

    var_info_t() = default;
    ~var_info_t() = default;
};

class scope_t {
    std::unordered_map<std::string, var_info_t*> data;

    int total_size;
    int base_offset;

public:
    
    const bool inherit_scope;
    
    scope_t();
    ~scope_t();
    scope_t(bool _inherit_scope, int _base_offset);
    
    int size();
    int get_end_offset();
    
    var_info_t* at(const std::string& key);
    var_info_t* operator[](const std::string& key);

    void add(const std::string& name, var_info_t* varinfo);
};

class symbol_table_t {
    std::deque<scope_t*> scope_stack;
    scope_name_allocator_t name_alloc;

public:
    symbol_table_t();
    ~symbol_table_t();

    // Get variable info
    var_info_t* get(const std::string& key);
    var_info_t* operator[](const std::string& key);

    // Add a variable to the current scope
    std::string add(const std::string& name, const int type, addr_info_t* addr);
    
    bool is_global_scope();

    void push_scope(bool inherit_scope);
    void pop_scope();
};

#endif
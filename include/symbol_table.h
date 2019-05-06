#ifndef COM_SCOPE_H
#define COM_SCOPE_H

#include <unordered_map>
#include <string>
#include <deque>

#include "parser_types.h"

struct addr_info_t {
    
    virtual std::string get_address_string() = 0;
    virtual ~addr_info_t() = default;
};

struct global_addr_info_t : addr_info_t {
    
    std::string label;

    ~global_addr_info_t() = default;
    
    global_addr_info_t(const std::string& _label) : label(_label) { }
    std::string get_address_string() override;
};

struct local_addr_info_t : addr_info_t {

    int base_offset;
    
    ~local_addr_info_t() = default;

    local_addr_info_t(int _base_offset) : base_offset(_base_offset) { }
    std::string get_address_string() override;
};

class scope_name_allocator_t {
    std::unordered_map<std::string, int> name_counter;

public:
    scope_name_allocator_t();
    std::string get_name(const std::string& id);
};

class label_allocator_t {
    int counter;

public:
    label_allocator_t();
    std::string get_label_name();
};

class scope_t;

struct var_info_t {
    std::string name;
    std::string id;
    int type;
    addr_info_t* address; // Relative address to the base pointer ?
    scope_t* scope;

    var_info_t() = default;
    ~var_info_t() = default;

    bool operator==(const var_info_t& other);
};

struct alignment_info_t {
    int param_index = 0;
    int alignment = 0;

    alignment_info_t(int _index, int _alignment) : param_index(_index), alignment(_alignment) { }
};

struct func_info_t {

    std::string identifier;
    int return_type;
    bool defined;
    std::vector<var_info_t> param_vector;
    std::vector<alignment_info_t> alignment_vector;
    
    // Size occupied by parameters in bytes
    int params_size;
    int total_stack_size;

    func_info_t() = default;
    func_info_t(const func_decl_t* decl, translator_t* t);

    int get_alignment(int param_index);

    bool operator==(const func_info_t& other);
    bool operator!=(const func_info_t& other);
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

    void pop(int size);
    void push(int size);
    
    int size();
    int get_base_offset();
    int get_end_offset();

    int align(int size_to_align_to);
    
    var_info_t* at(const std::string& key);
    var_info_t* operator[](const std::string& key);

    void add(const std::string& name, int size, var_info_t* varinfo);

    void remove(const std::string &name);
};

class symbol_table_t {
    std::deque<scope_t*> scope_stack;
    std::unordered_map<std::string, func_info_t*> function_table;

    scope_name_allocator_t name_alloc;

public:
    symbol_table_t();
    ~symbol_table_t();

    // Get variable info
    var_info_t* get_var(const std::string& key);

    // Add a variable to the current scope
    var_info_t* add_var(const std::string& name, const int type, const int size, addr_info_t* addr);

    // Get function info
    func_info_t* get_func(const std::string& name);

    // Remove a function declaration/definition
    void remove_func(const std::string& name);

    // Add function to the global scope
    void add_func(const std::string& name, func_info_t* f);
    
    bool is_scope_reachable(scope_t* scope);
    bool is_global_scope();
    scope_t* get_current_scope();

    void push_scope(bool inherit_scope);
    void pop_scope();
};

#endif
#ifndef COM_TYPE_TABLE_H
#define COM_TYPE_TABLE_H

#include <string>
#include <utility>
#include <unordered_map>

struct type_descriptor_t {
    std::string name;
    int size;
};

class type_table_t {
    
    std::unordered_map<int, type_descriptor_t> type_map;

public:
    type_table_t();
    ~type_table_t();

    type_descriptor_t* at(int key);
    void insert(int key, type_descriptor_t value);
};

#endif
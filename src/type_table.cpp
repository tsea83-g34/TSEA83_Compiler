
#include "type_table.h"

type_table_t::type_table_t() {
    type_map = std::unordered_map<std::string, type_descriptor_t>();
    insert("char", (type_descriptor_t){"char", 1});
    insert("int", (type_descriptor_t){"int", 2});
    insert("long", (type_descriptor_t){"long", 4});
}

type_table_t::~type_table_t() {
    type_map.clear();
}

type_descriptor_t* type_table_t::at(std::string key) {
    if (type_map.count(key)) {
        return &type_map[key];
    } else return nullptr;
}

void type_table_t::insert(std::string key, type_descriptor_t value) {
    type_map.insert({key, value});
}
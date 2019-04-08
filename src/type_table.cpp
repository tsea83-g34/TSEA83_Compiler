
#include "type_table.h"

type_table_t::type_table_t() {
    type_map = std::unordered_map<int, type_descriptor_t>();
    insert(0, (type_descriptor_t){"char", 1});
    insert(1, (type_descriptor_t){"int", 2});
    insert(2, (type_descriptor_t){"long", 4});
}

type_table_t::~type_table_t() {
    type_map.clear();
}

type_descriptor_t* type_table_t::at(int key) {
    if (type_map.count(key)) {
        return &type_map[key];
    } else return nullptr;
}

void type_table_t::insert(int key, type_descriptor_t value) {
    type_map.insert({key, value});
}
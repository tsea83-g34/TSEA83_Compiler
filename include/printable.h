#ifndef COM_PRINTABLE_H
#define COM_PRINTABLE_H

#include <string>

struct printable_t {
    virtual std::string get_string() = 0;
};


#endif
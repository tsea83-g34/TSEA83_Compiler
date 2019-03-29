#ifndef COM_INTERFACES_H
#define COM_INTERFACES_H

#include <string>
#include <vector>

#include "parser.h"
#include "lexer.h"


struct printable_t {
    virtual std::string get_string() = 0;
};

struct undoable_t {
    std::vector<lex::token*> tokens;
    virtual void undo(parser_t* p);
    virtual void clear();
};


#endif
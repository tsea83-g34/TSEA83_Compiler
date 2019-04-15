#ifndef COM_INTERFACES_H
#define COM_INTERFACES_H

#include <string>
#include <vector>

#include "lexer.h"

class translator_t;

class parser_t;

struct printable_t {
    virtual std::string get_string(parser_t* p) = 0;
};

struct undoable_t {
    std::vector<lex::token*> tokens;
    virtual void undo(parser_t* p) = 0;
    
    undoable_t() = default;
    virtual ~undoable_t(){}
};

struct translateable_t {

    virtual int translate(translator_t* t) = 0;
};


#endif
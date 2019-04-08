#ifndef COM_TRANSLATOR_H
#define COM_TRANSLATOR_H

#include "symbol_table.h"

class translator_t {

public:
    symbol_table_t symbol_table;
    long instr_cnt;
};

#endif
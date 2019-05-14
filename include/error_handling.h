
#include <string>
#include <iostream>

#include "interfaces.h"

#ifndef COM_ERROR_HANDLING_H
#define COM_ERROR_HANDLING_H



void output_warning(const std::string& warning, const undoable_t* node);

int get_warning_count();

#endif
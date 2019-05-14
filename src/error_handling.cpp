
#include "../include/error_handling.h"

static int warning_count = 0;

void output_warning(const std::string& warning, const undoable_t* node) {

    warning_count++;

    int line = node->tokens.front()->line_number;
    int column = node->tokens.front()->column_number;
    std::cout << "\033[0;33m--- Warning\033[0m " << line << ":" << column << "\033[0;33m:\033[0m  " << warning << '\n' << std::endl;
}

int get_warning_count() {
    return warning_count;
}


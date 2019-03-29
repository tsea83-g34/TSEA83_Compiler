
#include "../include/interfaces.h"

void undoable_t::undo(parser_t* p) {
    while (!tokens.empty()) {
        p->put_back_token(tokens.back());
        tokens.pop_back();
    }
}

void undoable_t::clear() {
    while (!tokens.empty()) {
        delete tokens.back();
        tokens.pop_back();
    }
}
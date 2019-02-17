#include <iostream>
#include <cstring>

#ifdef WINDOWS
    #include <direct.h>
    #define get_current_dir _getcwd
#else
    #include <unistd.h>
    #define get_current_dir getcwd
#endif

#include "../include/lexer.h"

using namespace std;


int main(int argc, char const *argv[]) {
    
    // Get current directory
    char current_path[100];
    memset(current_path, sizeof(current_path), 0);
    get_current_dir(current_path, sizeof(current_path));

    // Get file path from command line args
    string path = string(current_path);
    string relative_path(argv[1]);

    cerr << "wd: " << path << endl;
    cerr << "rel. path: " << relative_path << endl;   
    
    // Concatenate paths depending on system separator
    #ifdef WINDOWS
        string filename = path + "\\" + relative_path;
    #else
        string filename = path + '/' + relative_path;
    #endif
    cout << "total: " << filename << endl;

    lexer lex(filename);

    token* t = lex.get_next_token();
    while (t->tag != tag_t::eof) {

        switch(t->tag) {
            case tag_t::IF:
                cout << "If found" << endl;
                break;
            case tag_t::ID:
                cout << "Identifier found: " << ((id_token*) t)->lexeme << endl;
                delete t;
                break;
            case tag_t::INT_LITERAL:
                cout << "Int literal found: " << ((int_literal_token*) t)->value << endl;
                delete t;
                break;
            case tag_t::STRING_LITERAL:
                cout << "String literal found: " << ((str_literal_token*) t)->value << endl;
                delete t;
                break;
        }

        t = lex.get_next_token();
    }

    return 0;
}

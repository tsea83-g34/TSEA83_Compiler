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

    std::ios_base::sync_with_stdio(false);
    
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
    int token_count = 1;
    token* t = lex.get_next_token();
    while (t->tag != tag_t::eof) {
        
        cout << "Token " << token_count++ << ": ";
        switch(t->tag) {
            case tag_t::IF:
                cout << "If found" << endl;
                break;
            case tag_t::ID:
                cout << "Identifier found: " << dynamic_cast<id_token*>(t)->lexeme << endl;
                break;
            case tag_t::INT_LITERAL:
                cout << "Int literal found: " << dynamic_cast<int_literal_token*>(t)->value << endl;
                break;
            case tag_t::STRING_LITERAL:
                cout << "String literal found: " << dynamic_cast<str_literal_token*>(t)->value << endl;
                break;
            case tag_t::PLUS:
                cout << "Plus operator found" << endl;
                break;
            case tag_t::MINUS:
                cout << "Minus operator found" << endl;
                break;
            case tag_t::EQUALS:
                cout << "Equality relational operator found" << endl;
                break;
            case tag_t::NOT_EQUALS:
                cout << "Non-equality relational operator found" << endl;
                break;
            case tag_t::OPEN_PAREN:
                cout << "Open parenthesis found" << endl;
                break;
            case tag_t::CLOSED_PAREN:
                cout << "Closed parenthesis found" << endl;
                break;
            case tag_t::OPEN_BRACE:
                cout << "Open brace found" << endl;
                break;
            case tag_t::CLOSED_BRACE:
                cout << "Closed brace found" << endl;
                break;
            case tag_t::ASSIGNMENT:
                cout << "Assignment operator found" << endl;
                break;
            case tag_t::SEMI_COLON:
                cout << "Semi-colon found" << endl;
                break;
            case tag_t::UNKNOWN:
                cout << "UNKNOWN token found!" << endl;
        }
        delete t;
        t = lex.get_next_token();
    }

    return 0;
}

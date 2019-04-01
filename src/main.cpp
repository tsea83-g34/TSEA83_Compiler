#include <iostream>
#include <cstring>

#ifdef WINDOWS
    #include <direct.h>
    #define get_current_dir _getcwd
#else
    #include <unistd.h>
    #define get_current_dir getcwd
#endif

#include "../include/parser_types.h"
#include "../include/lexer.h"
#include "../include/parser.h"

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

    lex::lexer lex(filename);
    parser_t parser(&lex);

    program_t* program = parser.parse_token_stream();
    cout << "Finished parsing" << endl;
    if (program != nullptr) {
        cout << program->get_string(&parser) << endl;
        program->undo(&parser);
        delete program;
    } else {
        cout << "Syntax error" << endl;
    }

    #if 0
    int token_count = 1;
    lex::token* t = lex.get_next_token();
    while (t->tag != lex::tag_t::eof) {
        
        cout << "Token " << token_count++ << ": ";
        switch(t->tag) {
            case lex::tag_t::IF:
                cout << "If found" << endl;
                break;
            case lex::tag_t::ID:
                cout << "Identifier found: " << dynamic_cast<lex::id_token*>(t)->lexeme << endl;
                break;
            case lex::tag_t::INT_LITERAL:
                cout << "Int literal found: " << dynamic_cast<lex::int_literal_token*>(t)->value << endl;
                break;
            case lex::tag_t::STRING_LITERAL:
                cout << "String literal found: " << dynamic_cast<lex::str_literal_token*>(t)->value << endl;
                break;
            case lex::tag_t::PLUS:
                cout << "Plus operator found" << endl;
                break;
            case lex::tag_t::MINUS:
                cout << "Minus operator found" << endl;
                break;
            case lex::tag_t::EQUALS:
                cout << "Equality relational operator found" << endl;
                break;
            case lex::tag_t::NOT_EQUALS:
                cout << "Non-equality relational operator found" << endl;
                break;
            case lex::tag_t::OPEN_PAREN:
                cout << "Open parenthesis found" << endl;
                break;
            case lex::tag_t::CLOSED_PAREN:
                cout << "Closed parenthesis found" << endl;
                break;
            case lex::tag_t::OPEN_BRACE:
                cout << "Open brace found" << endl;
                break;
            case lex::tag_t::CLOSED_BRACE:
                cout << "Closed brace found" << endl;
                break;
            case lex::tag_t::ASSIGNMENT:
                cout << "Assignment operator found" << endl;
                break;
            case lex::tag_t::SEMI_COLON:
                cout << "Semi-colon found" << endl;
                break;
            case lex::tag_t::UNKNOWN:
                cout << "UNKNOWN token found!" << endl;
        }
        delete t;
        t = lex.get_next_token();
    }
    #endif
    return 0;
}

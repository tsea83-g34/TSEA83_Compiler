#include <iostream>
#include <cstring>
#include <fstream>

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
#include "../include/translator.h"
#include "../include/error_handling.h"

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
    
    // Concatenate paths depending on system separator
    #ifdef WINDOWS
        string filename = path + "\\" + relative_path;
    #else
        string filename = path + '/' + relative_path;
    #endif

    lex::lexer lex(filename);

    parser_t parser(&lex);
    program_t* program = nullptr;

    cout << endl;

    try {
        program = parser.parse_token_stream();
    } catch (syntax_error e) {
        cout << e.what() << std::endl;
        return 0;
    }
    
    if (program != nullptr) {
        cout << "\033[0;32mParsing successful. Abstract syntax tree: \033[0m" << endl;
        cout << program->get_string(&parser) << endl;
    }

    cout << endl;

    translator_t translator = translator_t();

    try {
        program->translate(&translator);
    } catch (translation_error e) {
        cout << e.what() << std::endl;
        return 0;
    }

    cout << "\033[0;32mCompiled with " << get_warning_count() << " warning(s) and no errors. \033[0m" << endl << endl;

    ofstream output_file("output.a");
    translator.print_to_file(output_file);

    return 0;
}

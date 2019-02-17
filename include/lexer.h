#ifndef COM_LEXER_H
#define COM_LEXER_H

#include <unordered_map>
#include <string>
#include <fstream>
#include <regex>

#include "tokens.h"

#define BUFFER_SIZE 4096
#define READ_SIZE BUFFER_SIZE - 1

class lexer {
private:

    // Keyword map
    std::unordered_map<std::string, keyword_token*> reserved_words;
    
    // Input stream and double buffer
    std::ifstream file;
    char *buffer_1;
    char *buffer_2;

    // True if currently on buffer 1, false if on buffer 2
    bool buffer_switch;

    // Pointers for reading
    char *lexemeBegin;
    char *forward;

    // Current line
    int line;
    
    // Regular expressions
    std::regex identifier_regex;
    std::regex int_literal_regex;
    std::regex str_literal_regex;
    std::regex whitespace_regex;

    // Converts an ASCII digit character to its corresponding integer digit
    int char_to_digit(char c);
    
    void switch_buffer();

public:
    lexer(std::string filename);

    // Acquires the next token from the buffer
    token* get_next_token();

};

#endif
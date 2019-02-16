#ifndef COM_LEXER_H
#define COM_LEXER_H

#include <unordered_map>
#include <string>
#include <fstream>
#include <regex>

#include "tokens.h"

#define BUFFER_SIZE 4096

class lexer {
private:

    // Keyword map
    std::unordered_map<std::string, keyword_token> reserved_words;
    
    // Input stream and buffer
    std::ifstream file;
    char *buffer;

    // Current line
    int line;
    
    // Regular expressions
    std::regex identifier_regex;
    std::regex int_literal_regex;
    std::regex str_literal_regex;
    std::regex whitespace_regex;


    void reserve(keyword_token&& w);

    int char_to_digit(char c);
    
public:
    lexer(std::string filename);

    token get_next_token();

};

#endif
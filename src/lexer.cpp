
#include "../include/lexer.h"

#include <utility>
#include <cctype>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <cstring>

using namespace lex;

lexer::lexer(std::string filename) {

    reserved_words = std::unordered_map<std::string, tag_t>();
    line = 0;

    identifier_regex        = std::regex("([A-Z]|[a-z]|_)(([A-Z]|[a-z]|_)|[0-9])*");
    int_literal_regex       = std::regex("([1-9][0-9]*)|0");
    str_literal_regex       = std::regex("\"([^\"]*)\"");
    whitespace_regex        = std::regex("[\t\n\ ]+");
    half_str_literal_regex  = std::regex("\"([^\"]*)");

    // Open file, allocate buffer memory and read BUFFER_SIZE characters
    file = std::ifstream(filename);
    buffer_1 = (char*) calloc(BUFFER_SIZE, sizeof(char));
    buffer_1_end = buffer_1 + BUFFER_SIZE - 1;

    buffer_2 = (char*) calloc(BUFFER_SIZE, sizeof(char));
    buffer_2_end = buffer_2 + BUFFER_SIZE - 1;

    //buffer_1[READ_SIZE] = '\0';
    //buffer_2[READ_SIZE] = '\0';

    // Read to buffer_1 and set bufferswitch
    file.read(buffer_1, READ_SIZE);
    buffer_switch = true;

    // Set pointers
    lexeme_start = buffer_1;

    // Reserve keywords
    reserved_words.insert({"if", tag_t::IF});
    reserved_words.insert({"return", tag_t::RETURN});
    reserved_words.insert({"while", tag_t::WHILE});
}

lexer::~lexer() {
    free(buffer_1);
    free(buffer_2);
}

int lexer::char_to_digit(char c) {
    return c - '0';
}

char* lexer::get_current_buffer() {
    return (buffer_switch) ? buffer_1 : buffer_2;
}

const char* lexer::get_current_buffer_end() {
    return (buffer_switch) ? buffer_1_end : buffer_2_end;
}

char* lexer::switch_buffer() {
    // Overwrite the non-current buffer and switch to it
    char* other_buffer = (buffer_switch) ? buffer_2 : buffer_1;
    file.read(other_buffer, READ_SIZE);
    buffer_switch = !buffer_switch;

    // Set pointers
    lexeme_start = other_buffer;
    return other_buffer;
}

void lexer::handle_split(std::regex r, std::string& result, unsigned int size) {
    // Allocate a local buffer that will be returned
    char *local_buffer = (char*) calloc(MAX_TOKEN_SIZE, sizeof(char));

    // Copy the beginning of the lexeme from the old buffer into the local one
    std::strcpy(local_buffer, lexeme_start);

    // Read from file, switch buffer and read the beginning of the new buffer into the local one
    char *new_buffer = switch_buffer();
    std::memcpy(local_buffer+size, new_buffer, MAX_TOKEN_SIZE - size - 1);
    
    // Find the complete token
    std::cmatch cm;
    std::regex_search(local_buffer, cm, r);

    // Move the lexeme pointer to the appropriate index
    lexeme_start += cm[0].length() - size;
    
    // Update the result and free the buffer
    result = cm[0].str();
    free(local_buffer);
}

token* lexer::get_next_token() {

    while (true) {
        // If eof return eof token
        if (*lexeme_start == '\0' && file.eof() || *lexeme_start == EOF) {
            return new token(tag_t::eof);
        // If lexeme_start reached end of buffer, switch buffer
        } else if (*lexeme_start == '\0') {
            // Time to switch buffer
            switch_buffer();
        }

        std::cmatch cm;
        // Look for whitespace
        if(std::regex_search(lexeme_start, cm, whitespace_regex) && cm.prefix().length() == 0) {
            lexeme_start += cm[0].length();
            std::string spaces = cm[0].str();
            line += std::count(spaces.begin(), spaces.end(), '\n');
            //std::cout << "Removed " << cm[0].length() << " whitespace characters" << std::endl;
            continue; // Try again in case of buffer switch
        }
        
        // Check for comments
        if (lexeme_start[0] == '/' && lexeme_start[1] == '/') {
            do {
                if (lexeme_start == get_current_buffer_end()) {
                    switch_buffer();
                    continue;
                }
            } while (*lexeme_start++ != '\n');
            line++;

            // Restart token matching loop
            continue;
        
        // Check case where comment signifier // is split between buffers
        } else if (*lexeme_start == '/' && lexeme_start+1 == get_current_buffer_end()) {
            switch_buffer();
            if (*lexeme_start == '/') {
                do {
                    if (lexeme_start == get_current_buffer_end()) {
                        switch_buffer();
                        continue;
                    }
                } while (*lexeme_start++ != '\n');
            } else {
                return new token(tag_t::UNKNOWN);
            }
        }

        // Check for identifiers
        if (std::regex_search(lexeme_start, cm, identifier_regex) && cm.prefix().length() == 0) {

            std::string word = cm[0].str();
            // If the token could potentially overflow to the next buffer
            if (lexeme_start + word.length() == get_current_buffer_end()) {
                
                handle_split(identifier_regex, word, word.length());
                
            // Otherwise just offset the pointer with the word length
            } else lexeme_start += word.length();
                
            // If found word is keyword
            if (reserved_words.count(word)) return new keyword_token(reserved_words[word]);

            // Otherwise create a new identifier token
            return new id_token(std::move(word));
        }

        // Check for integer literals
        if (std::regex_search(lexeme_start, cm, int_literal_regex) && cm.prefix().length() == 0) {
            std::string literal = cm[0].str();

            // If the token could potentially overflow to the next buffer
            if (lexeme_start + literal.length() == get_current_buffer_end()) {
             
                handle_split(int_literal_regex, literal, literal.length());
            
            // Otherwise just offset the pointer with the word length
            } else lexeme_start += literal.length();

            // Parse integer value and return new integer literal token
            int value = std::stoi(literal);
            return new int_literal_token(value);
        }

        // Check for string literals
        if (std::regex_search(lexeme_start, cm, str_literal_regex) && cm.prefix().length() == 0) {
            std::string literal = cm[0].str();
            lexeme_start += literal.length();

            return new str_literal_token(std::move(literal));
        // In case literal is split between buffers, look for half string
        } else if (std::regex_search(lexeme_start, cm, half_str_literal_regex) && cm.prefix().length() == 0) {
            
            std::string literal = cm[0].str();
            if (lexeme_start + literal.length() == get_current_buffer_end()) {
                handle_split(str_literal_regex, literal, literal.length());
            } else return new token(tag_t::UNKNOWN);

            return new str_literal_token(std::move(literal));
        }

        // Check for parenthesis, single character operators and semi-colons
        switch (*lexeme_start) {
            case ';':
                lexeme_start++;
                return new token(tag_t::SEMI_COLON);
            case '(':
                lexeme_start++;
                return new token(tag_t::OPEN_PAREN);
            case ')':
                lexeme_start++;
                return new token(tag_t::CLOSED_PAREN);
            case '{':
                lexeme_start++;
                return new token(tag_t::OPEN_BRACE);
            case '}':
                lexeme_start++;
                return new token(tag_t::CLOSED_BRACE);
            case '+':
                lexeme_start++;
                return new arithop_token(tag_t::PLUS);
            case '-':
                lexeme_start++;
                return new arithop_token(tag_t::MINUS);
        }

        // Check for relational operators
        if (*lexeme_start == '=') {
            // Handle split
            if (lexeme_start[1] == '\0') {
                switch_buffer();
            } else lexeme_start++;

            switch(*lexeme_start) {
                case '=':
                    lexeme_start++;
                    return new relop_token(tag_t::EQUALS);
                    break;
                default:
                    return new token(tag_t::ASSIGNMENT);
            }
        }

        if (*lexeme_start == '!') {
            // Handle split
            if (lexeme_start[1] == '\0') {
                switch_buffer();
            } else lexeme_start++;

            switch(*lexeme_start) {
                case '=':
                    lexeme_start++;
                    return new relop_token(tag_t::NOT_EQUALS);
                    break;
                default:
                    return new token(tag_t::UNKNOWN);
            }
        }

        return new token(tag_t::UNKNOWN);
    }
}
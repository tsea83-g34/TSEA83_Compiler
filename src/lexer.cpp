
#include "../include/lexer.h"

#include <utility>
#include <cctype>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <cstring>

lexer::lexer(std::string filename) {

    reserved_words = std::unordered_map<std::string, keyword_token*>();
    line = 0;

    identifier_regex    = std::regex("([A-Z]|[a-z]|_)(([A-Z]|[a-z]|_)|[0-9])*");
    int_literal_regex   = std::regex("[1-9][0-9]*");
    str_literal_regex   = std::regex("\"([^\"]*)\"");
    whitespace_regex    = std::regex("[\t\n\ ]+");

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
    forward = buffer_1;

    // Reserve keywords
    reserved_words.insert({"if", new keyword_token(tag_t::IF)});
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
    forward = other_buffer;
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

    // TODO: Handle cases where tokens are split between buffers

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
            continue; // Try again in case of buffer switch
        }
        
        // Check for identifiers
        if (std::regex_search(lexeme_start, cm, identifier_regex) && cm.prefix().length() == 0) {

            std::string word = cm[0].str();
            // If the token could potentially overflow to the next buffer
            if (lexeme_start + word.length() == get_current_buffer_end()) {
                
                handle_split(identifier_regex, word, word.length());
                
            // Otherwise just offset the pointer with the word length
            } else lexeme_start += word.length();
                
            // If found word is keyword, return the already created token
            if (reserved_words.count(word)) return reserved_words[word];

            // Otherwise create a new identifier token
            return new id_token(std::move(word));
        }

        // Check for integer literals
        if (std::regex_search(lexeme_start, cm, int_literal_regex) && cm.prefix().length() == 0) {
            std::string literal = cm[0].str();
            lexeme_start += literal.length();

            int value = std::stoi(literal);
            return new int_literal_token(value);
        }

        // Check for string literals
        if (std::regex_search(lexeme_start, cm, str_literal_regex) && cm.prefix().length() == 0) {
            std::string literal = cm[0].str();
            lexeme_start += literal.length();

            return new str_literal_token(std::move(literal));
        }

        return new token(tag_t::UNKNOWN);
    }
}

#if 0

token* get_next_token_old() {

    char next;

    if (file.eof()) return new token(tag_t::eof);

    for (;; next = file.get()) {

        if (next == ' ' || next == '\t') continue;
        else if (next == '\n') {
            line++;
            continue;
        }

        // Comments
        if (next == '/') {
            // Line comment
            if (file.peek() == '/') do {
                next = file.get();
            } while (next != '\n');
            // Block comment
            else if (next == '*') do {
                next = file.get();
                if (next == '\n') line++;
            } while (next != '*' || file.peek() != '/');
        } else if (next == -1) return new token(tag_t::eof);
        
        // Check for integer literal
        if (isdigit(next)) {
            int v = 0;
            do {
                v = 10 * v + char_to_digit(next);
            } while (isdigit(next));
            return new int_literal_token(v);
        }

        // Check for string literal
        if (next == '\"') {
            std::stringstream buffer{};
            do {
                buffer << next;
                next = file.get();
            } while (next != '\"');
            buffer << next; // Append second quotation
            return new str_literal_token(buffer.str());
        }

        // Check for identifier or keyword
        if (isalpha(next)) {
            std::stringstream buffer{};
            do {
                buffer << next;
                next = file.get();
            } while(isalnum(next));

            std::string word = buffer.str();
            if (reserved_words.count(word)) return reserved_words[word];

            // identifier
            return new id_token(std::move(word));
        }

        std::cout << "Error, unidentified token!" << std::endl;
    }
}

#endif
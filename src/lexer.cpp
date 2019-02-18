
#include "../include/lexer.h"

#include <utility>
#include <cctype>
#include <sstream>
#include <iostream>

lexer::lexer(std::string filename) {

    reserved_words = std::unordered_map<std::string, keyword_token*>();
    line = 0;

    identifier_regex    = std::regex("([A-Z]|[a-z]|_)(([A-Z]|[a-z]|_)|[0-9])*");
    int_literal_regex   = std::regex("[1-9][0-9]*");
    str_literal_regex   = std::regex("\"([^\"]*)\"");
    whitespace_regex    = std::regex("[\t\n\ ]*");

    // Open file, allocate buffer memory and read BUFFER_SIZE characters
    file = std::ifstream(filename);
    buffer_1 = new char[BUFFER_SIZE];
    buffer_2 = new char[BUFFER_SIZE];

    buffer_1[READ_SIZE] = '\0';
    buffer_2[READ_SIZE] = '\0';

    // Read to buffer_1 and set bufferswitch
    file.read(buffer_1, READ_SIZE);
    buffer_switch = true;

    // Set pointers
    lexemeBegin = buffer_1;
    forward = buffer_1;

    // Reserve keywords
    reserved_words.insert({"if", new keyword_token(tag_t::IF)});
}

int lexer::char_to_digit(char c) {
    return c - '0';
}

void lexer::switch_buffer() {
    // Overwrite the non-current buffer and switch to it
    char* other_buffer = (buffer_switch) ? buffer_2 : buffer_1;
    file.read(other_buffer, READ_SIZE);
    buffer_switch = !buffer_switch;

    // Set pointers
    lexemeBegin = other_buffer;
    forward = other_buffer;
}

token* lexer::get_next_token() {

    // Derive which buffer is currently being read
    char* current_buffer = (buffer_switch) ? buffer_1 : buffer_2;

    // If eof return eof token
    if (*lexemeBegin == '\0' && file.eof() || *lexemeBegin == EOF) {
        return new token(tag_t::eof);

    // If forward reached end of buffer, switch buffer
    } else if (*lexemeBegin == '\0') {
        // Time to switch buffer
        switch_buffer();
    }

    std::cmatch cm;
    // Look for whitespace
    if(std::regex_search(lexemeBegin, cm, whitespace_regex) && cm.prefix().length() == 0) {
        lexemeBegin += cm[0].length();
    }
    //cm = std::cmatch();

    // Check for string literals
    if (std::regex_search(lexemeBegin, cm, str_literal_regex) && cm.prefix().length() == 0) {
        std::string literal = cm[0];
        lexemeBegin += literal.length();

        return new str_literal_token(std::move(literal));
    }
    //cm = std::cmatch();
    
    // Check for identifiers
    if (std::regex_search(lexemeBegin, cm, identifier_regex) && cm.prefix().length() == 0) {
        std::string word = cm[0];
        lexemeBegin += word.length();
            
        // If found word is keyword, return the already created token
        if (reserved_words.count(word)) return reserved_words[word];

        // Otherwise create a new identifier token
        return new id_token(std::move(word));
    }
    //cm = std::cmatch();

    // Check for integer literals
    if (std::regex_search(lexemeBegin, cm, int_literal_regex) && cm.prefix().length() == 0) {
        std::string literal = cm[0];
        lexemeBegin += literal.length();

        int value = std::stoi(literal);
        return new int_literal_token(value);
    }
    //cm = std::cmatch();
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
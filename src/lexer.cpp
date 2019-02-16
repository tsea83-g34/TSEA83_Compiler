
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
    buffer = new char[BUFFER_SIZE];
    file.read(buffer, BUFFER_SIZE);

    // Reserve keywords
    reserved_words.insert({"if", new keyword_token(tag_t::IF)});
}

int lexer::char_to_digit(char c) {
    return c - '0';
}

token* lexer::get_next_token() {

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
#ifndef COM_TOKENS_H
#define COM_TOKENS_H

#include <string>

enum class tag_t : int {
    
    // Keywords
    IF,

    // Other
    ID,
    INT_LITERAL,
    STRING_LITERAL,
    
    // Special characters
    OPEN_PARAN,
    CLOSED_PARAN,
    OPEN_BRACKET,
    CLOSED_BRACKET,
    
    eof
};

struct token {
    const tag_t tag;
    token(tag_t _tag) : tag(_tag) { }
};

struct id_token : token {
    std::string lexeme;
    id_token(const std::string& _lexeme) : token(tag_t::ID) { lexeme = _lexeme; }
    id_token(const std::string&& _lexeme) : token(tag_t::ID) { lexeme = _lexeme; }
};

struct keyword_token : token {
    keyword_token(tag_t _tag) : token(_tag) { }
};

struct int_literal_token : token {
    int value;
    int_literal_token(int _value) : token(tag_t::INT_LITERAL), value(_value) {}
};

struct str_literal_token : token {
    std::string value;
    str_literal_token(const std::string& _value) : token(tag_t::STRING_LITERAL) { value = _value; }
    str_literal_token(const std::string&& _value) : token(tag_t::STRING_LITERAL) { value = _value; }
};

#endif
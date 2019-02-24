#ifndef COM_TOKENS_H
#define COM_TOKENS_H

#include <string>

/* Enum for token group identifiers */
enum class type_t : int {
    
    GENERAL,
    KEYWORD,
    IDENTIFIER,
    ASSIGNMENT,
    INT_LITERAL,
    STRING_LITERAL,
    ARITHMETIC_OPERATOR,
    RELATIONAL_OPERATOR
};

/* Enum for specific tokens */
enum class tag_t : int {
    
    // Keywords
    IF,

    // Other
    ID,
    INT_LITERAL,
    STRING_LITERAL,

    // Operators
    PLUS,
    MINUS,
    EQUALS,
    NOT_EQUALS,
    ASSIGNMENT,
    
    // Special characters
    OPEN_PAREN,
    CLOSED_PAREN,
    OPEN_BRACE,
    CLOSED_BRACE,
    
    eof,
    UNKNOWN
};

struct token {
    const tag_t tag;
    token(tag_t _tag) : tag(_tag) { }

    virtual type_t type() { return type_t::GENERAL; }
};

struct id_token : token {
    std::string lexeme;
    id_token(const std::string& _lexeme) : token(tag_t::ID) { lexeme = _lexeme; }
    id_token(const std::string&& _lexeme) : token(tag_t::ID) { lexeme = _lexeme; }

    virtual type_t type() override { return type_t::IDENTIFIER; }
};

struct keyword_token : token {
    keyword_token(tag_t _tag) : token(_tag) { }

    virtual type_t type() override { return type_t::KEYWORD; }
};

struct int_literal_token : token {
    int value;
    int_literal_token(int _value) : token(tag_t::INT_LITERAL), value(_value) {}

    virtual type_t type() override { return type_t::INT_LITERAL; }
};

struct str_literal_token : token {
    std::string value;
    str_literal_token(const std::string& _value) : token(tag_t::STRING_LITERAL) { value = _value; }
    str_literal_token(const std::string&& _value) : token(tag_t::STRING_LITERAL) { value = _value; }

    virtual type_t type() override { return type_t::STRING_LITERAL; }
};

struct arithop_token : token {
    arithop_token(tag_t _tag) : token(_tag) { }

    virtual type_t type() override { return type_t::ARITHMETIC_OPERATOR; }
};

struct relop_token : token {
    relop_token(tag_t _tag) : token(_tag) { }

    virtual type_t type() override { return type_t::RELATIONAL_OPERATOR; }
};

#endif
#ifndef COM_TOKENS_H
#define COM_TOKENS_H

#include <string>

namespace lex {

    /* Enum for specific tokens */
    enum class tag_t : int {
        
        // Keywords
        IF,
        RETURN,
        WHILE,
        ELSE,
        ASM,

        // Other
        ID,
        INT_LITERAL,
        STRING_LITERAL,

        // Operators
        PLUS,
        MINUS,
        AND,
        OR,
        NOT,
        EQUALS,
        NOT_EQUALS,
        LESS,
        GREATER,
        LESS_OR_EQUAL,
        GREATER_OR_EQUAL,
        ASSIGNMENT,
        
        // Special characters
        STAR,
        OPEN_PAREN,
        CLOSED_PAREN,
        OPEN_BRACE,
        CLOSED_BRACE,
        SEMI_COLON,
        
        eof,
        UNKNOWN
    };

    const std::string token_names[] = {
        "if keyword", 
        "return keyword",
        "while keyowrd",
        "else keyword",
        "asm keyword",
        "identifier", 
        "integer literal", 
        "string literal",
        "plus operator",
        "minus operator",
        "& operator",
        "or operator",
        "not operator",
        "equals operator",
        "not equals operator",
        "less operator",
        "greater operator",
        "less or equal operator",
        "greater or equal operator",
        "assignment operator",
        "* operator",
        "open parenthesis",
        "closed parenthesis",
        "open brace",
        "closed brace",
        "semicolon",
        "end of file",
        "unknown"
    };

    struct token {
        const tag_t tag;
        token(tag_t _tag) : tag(_tag) { }
        int line_number;
        int column_number;
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

}

#endif
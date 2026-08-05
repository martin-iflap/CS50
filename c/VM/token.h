#ifndef TOKEN_H
#define TOKEN_H


// -------------------------------------------  TOKEN DEFINITIONS  -------------------------------------------

typedef enum
{
    /* Literals */
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING, // 2

    /* Keywords */
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_PRINT,
    TOKEN_RETURN, // 7

    /* Booleans */
    TOKEN_TRUE,
    TOKEN_FALSE, // 9

    /* Arithmetic */
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_PERCENT, // 14

    /* Assignment */
    TOKEN_EQUAL, // 15

    /* Comparisons */
    TOKEN_EQUAL_EQUAL,
    TOKEN_BANG,
    TOKEN_BANG_EQUAL, // 18

    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL, // 20

    TOKEN_LESS,
    TOKEN_LESS_EQUAL, // 22

    /* Boolean operators (keywords later) */
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT, // 25

    /* Brackets */
    TOKEN_LPAREN,
    TOKEN_RPAREN, // 27

    TOKEN_LBRACE,
    TOKEN_RBRACE, // 29

    TOKEN_LBRACKET,
    TOKEN_RBRACKET, // 31

    TOKEN_COMMA,
    TOKEN_SEMICOLON, // 33

    TOKEN_UNKNOWN,
    TOKEN_NEWLINE,
    TOKEN_EOF // 36

} TokenType;

// -----------------------------------------  TOKEN  ---------------------------------------------------

typedef struct
{
    TokenType type;
    char lexeme[64];
    union
    {
        float number;
        bool boolean;
        char *string;
    } literal;
} Token;


#endif
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define MAX_LENGTH 120
#define MAX_TOKENS 128

// -------------------------------------------  TOKEN DEFINITIONS  -------------------------------------------

typedef enum
{
    /* Literals */
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,

    /* Keywords */
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_PRINT,

    /* Arithmetic */
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_PERCENT,

    /* Assignment */
    TOKEN_EQUAL,

    /* Comparisons */
    TOKEN_EQUAL_EQUAL,
    TOKEN_BANG,
    TOKEN_BANG_EQUAL,

    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,

    TOKEN_LESS,
    TOKEN_LESS_EQUAL,

    /* Boolean operators (keywords later) */
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,

    /* Brackets */
    TOKEN_LPAREN,
    TOKEN_RPAREN,

    TOKEN_LBRACE,
    TOKEN_RBRACE,

    TOKEN_LBRACKET,
    TOKEN_RBRACKET,

    TOKEN_COMMA,
    TOKEN_SEMICOLON,

    TOKEN_UNKNOWN,
    TOKEN_EOF

} TokenType;

// -----------------------------------------  TOKEN  ---------------------------------------------------

typedef struct
{
    TokenType type;
    const char *start; // pointer into the original source
    int length; // number of characters
} Token;

// --------------------------------------  LEXER  --------------------------------------

typedef struct
{
    const char *source;
    int start;
    int current;
    Token tokens[MAX_TOKENS];
    int token_count;
} Lexer;

// --------------------------------------  BASIC HELPERS  --------------------------------------

bool isAtEnd(Lexer *lexer)
{
    return lexer->source[lexer->current] == '\0';
}

char advance(Lexer *lexer)
{
    return lexer->source[lexer->current++];
}

char peek(Lexer *lexer)
{
    return lexer->source[lexer->current];
}

char peekNext(Lexer *lexer)
{
    if (isAtEnd(lexer))
        return '\0';
    return lexer->source[lexer->current + 1];
}

// --------------------------------------  TOKEN CREATION --------------------------------------  

void addToken(Lexer *lexer, TokenType type)
{
    if (lexer->token_count >= MAX_TOKENS)
    {
        printf("Too many tokens.\n");
        return;
    }

    Token *token = &lexer->tokens[lexer->token_count++];

    token->type = type;
    token->start = lexer->source + lexer->start;
    token->length = lexer->current - lexer->start;
}

// --------------------------------------  KEYWORDS  ----------------------------------------------------

TokenType keywordType(const char *text, int length)
{
    if (length == 2 && strncmp(text, "if", 2) == 0)
        return TOKEN_IF;

    if (length == 4 && strncmp(text, "else", 4) == 0)
        return TOKEN_ELSE;

    if (length == 5 && strncmp(text, "while", 5) == 0)
        return TOKEN_WHILE;

    if (length == 5 && strncmp(text, "print", 5) == 0)
        return TOKEN_PRINT;

    if (length == 3 && strncmp(text, "and", 3) == 0)
        return TOKEN_AND;

    if (length == 2 && strncmp(text, "or", 2) == 0)
        return TOKEN_OR;

    if (length == 3 && strncmp(text, "not", 3) == 0)
        return TOKEN_NOT;

    return TOKEN_IDENTIFIER;
}

// --------------------------------------  SCAN IDENTIFIER  ------------------------------------------

void scanIdentifier(Lexer *lexer)
{
    while (isalnum(peek(lexer)) || peek(lexer) == '_')
    {
        advance(lexer);
    }
    TokenType type =
        keywordType(
            lexer->source + lexer->start,
            lexer->current - lexer->start);

    addToken(lexer, type);
}

// --------------------------------------  SCAN NUMBER  -----------------------------------------

void scanNumber(Lexer *lexer)
{
    while (isdigit(peek(lexer)))
    {
        advance(lexer);
    }
    /* Decimal numbers */
    if (peek(lexer) == '.' && isdigit(peekNext(lexer)))
    {
        advance(lexer);
        while (isdigit(peek(lexer)))
        {
            advance(lexer);
        }
    }
    addToken(lexer, TOKEN_NUMBER);
}

// --------------------------------------  SCAN ONE TOKEN  ---------------------------------------------

void scanToken(Lexer *lexer)
{
    char c = advance(lexer);
    switch (c)
    {
        /* Ignore whitespace */
        case ' ':
        case '\r':
        case '\t':
        case '\n':
            break;

        /* Single-character tokens */
        case '(':
            addToken(lexer, TOKEN_LPAREN);
            break;

        case ')':
            addToken(lexer, TOKEN_RPAREN);
            break;

        case '{':
            addToken(lexer, TOKEN_LBRACE);
            break;

        case '}':
            addToken(lexer, TOKEN_RBRACE);
            break;

        case '[':
            addToken(lexer, TOKEN_LBRACKET);
            break;

        case ']':
            addToken(lexer, TOKEN_RBRACKET);
            break;

        case ',':
            addToken(lexer, TOKEN_COMMA);
            break;

        case ';':
            addToken(lexer, TOKEN_SEMICOLON);
            break;

        case '+':
            addToken(lexer, TOKEN_PLUS);
            break;

        case '-':
            addToken(lexer, TOKEN_MINUS);
            break;

        case '*':
            addToken(lexer, TOKEN_STAR);
            break;

        case '/':
            addToken(lexer, TOKEN_SLASH);
            break;

        case '%':
            addToken(lexer, TOKEN_PERCENT);
            break;

        /* Two-character operators */
        case '=':
            if (peek(lexer) == '=')
            {
                advance(lexer);
                addToken(lexer, TOKEN_EQUAL_EQUAL);
            }
            else
            {
                addToken(lexer, TOKEN_EQUAL);
            }
            break;

        case '!':
            if (peek(lexer) == '=')
            {
                advance(lexer);
                addToken(lexer, TOKEN_BANG_EQUAL);
            }
            else
            {
                addToken(lexer, TOKEN_BANG);
            }
            break;

        case '>':
            if (peek(lexer) == '=')
            {
                advance(lexer);
                addToken(lexer, TOKEN_GREATER_EQUAL);
            }
            else
            {
                addToken(lexer, TOKEN_GREATER);
            }
            break;

        case '<':
            if (peek(lexer) == '=')
            {
                advance(lexer);
                addToken(lexer, TOKEN_LESS_EQUAL);
            }
            else
            {
                addToken(lexer, TOKEN_LESS);
            }
            break;

        /* String literal */
        case '"':
        case '\'':
            char quoteChar = lexer->source[lexer->current - 1];
            
            while (!isAtEnd(lexer) && peek(lexer) != quoteChar)
            {
                advance(lexer);
            }

            if (isAtEnd(lexer))
            {
                printf("Lexer Error: Unterminated string.\n");
                return;
            }
            advance(lexer);
            addToken(lexer, TOKEN_STRING);

            break;

        default:
            if (isdigit(c))
            {
                scanNumber(lexer);
            }

            else if (isalpha(c) || c == '_')
            {
                scanIdentifier(lexer);
            }

            else
            {
                addToken(lexer, TOKEN_UNKNOWN);
            }
    }
}

// --------------------------------------  LEXICAL ANALYZER  ----------------------------------------

void lexicalAnalyzer(Lexer *lexer)
{
    while (!isAtEnd(lexer))
    {
        lexer->start = lexer->current;

        scanToken(lexer);
    }
    lexer->start = lexer->current;
    addToken(lexer, TOKEN_EOF);
}

// --------------------------------------  DEBUG  ----------------------------------------------------

const char *tokenName(TokenType type)
{
    switch (type)
    {
        case TOKEN_IDENTIFIER:      return "IDENTIFIER";
        case TOKEN_NUMBER:          return "NUMBER";
        case TOKEN_STRING:          return "STRING";

        case TOKEN_IF:              return "IF";
        case TOKEN_ELSE:            return "ELSE";
        case TOKEN_WHILE:           return "WHILE";
        case TOKEN_PRINT:           return "PRINT";

        case TOKEN_AND:             return "AND";
        case TOKEN_OR:              return "OR";
        case TOKEN_NOT:             return "NOT";

        case TOKEN_PLUS:            return "PLUS";
        case TOKEN_MINUS:           return "MINUS";
        case TOKEN_STAR:            return "STAR";
        case TOKEN_SLASH:           return "SLASH";
        case TOKEN_PERCENT:         return "PERCENT";

        case TOKEN_EQUAL:           return "EQUAL";
        case TOKEN_EQUAL_EQUAL:     return "EQUAL_EQUAL";

        case TOKEN_BANG:            return "BANG";
        case TOKEN_BANG_EQUAL:      return "BANG_EQUAL";

        case TOKEN_GREATER:         return "GREATER";
        case TOKEN_GREATER_EQUAL:   return "GREATER_EQUAL";

        case TOKEN_LESS:            return "LESS";
        case TOKEN_LESS_EQUAL:      return "LESS_EQUAL";

        case TOKEN_LPAREN:          return "LPAREN";
        case TOKEN_RPAREN:          return "RPAREN";

        case TOKEN_LBRACE:          return "LBRACE";
        case TOKEN_RBRACE:          return "RBRACE";

        case TOKEN_LBRACKET:        return "LBRACKET";
        case TOKEN_RBRACKET:        return "RBRACKET";

        case TOKEN_COMMA:           return "COMMA";
        case TOKEN_SEMICOLON:       return "SEMICOLON";

        case TOKEN_UNKNOWN:         return "UNKNOWN";
        case TOKEN_EOF:             return "EOF";
    }

    return "INVALID";
}

void printTokens(Lexer *lexer)
{
    printf("\n========== TOKENS ==========\n\n");

    for (int i = 0; i < lexer->token_count; i++)
    {
        Token *token = &lexer->tokens[i];

        printf("%-18s  ->  \"%.*s\"\n",
               tokenName(token->type),
               token->length,
               token->start);
    }
}

// --------------------------------------  MAIN  ------------------------------------------------------

int main(void)
{
    char source[MAX_LENGTH] =
        "if x > 5 {print('x is greater')} else {print('x is smaller')}";

    Lexer lexer =
    {
        .source = source,
        .start = 0,
        .current = 0,
        .token_count = 0
    };
    lexicalAnalyzer(&lexer);
    printTokens(&lexer);
    return 0;
}

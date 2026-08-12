#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../token.h"


// --------------------------------------  LEXER  --------------------------------------

typedef struct
{
    const char *source;
    int start;
    int current;
    Token *tokens;
    int tokens_capacity;
    int token_count;
} Lexer;

// --------------------------------------  BASIC HELPERS  --------------------------------------

static bool isAtEnd(Lexer *lexer)
// check if current is at end of source
{
    return lexer->source[lexer->current] == '\0';
}

static char advance(Lexer *lexer)
// return current char from source and advance current + 1
{
    return lexer->source[lexer->current++];
}

static char peek(Lexer *lexer)
// return the current char from source
{
    return lexer->source[lexer->current];
}

static char peekNext(Lexer *lexer)
// peek at the next token after current and make sure its not the end
{
    if (isAtEnd(lexer))
        return '\0';

    if (lexer->source[lexer->current + 1] == '\0')
        return '\0';

    return lexer->source[lexer->current + 1];
}

static void consumeComment(Lexer *lexer)
// consume the entire comment
{
    char c = advance(lexer);
    while(c != '\n' && !isAtEnd(lexer))
    {
        c = advance(lexer);
    }
}

// --------------------------------------  TOKEN CREATION --------------------------------------  

static Token *addToken(Lexer *lexer, TokenType type)
// add token to the tokens array and increment token count
{
    if(lexer->token_count >= lexer->tokens_capacity) // double the capacity if needed
    {
        lexer->tokens_capacity *= 2;
        lexer->tokens = realloc(lexer->tokens, lexer->tokens_capacity * sizeof(Token));
    }

    Token *token = &lexer->tokens[lexer->token_count++];

    token->type = type;

    memset(&token->literal, 0, sizeof(token->literal)); // initialize all fields to 0s

    int len = lexer->current - lexer->start;
    if (len >= sizeof(token->lexeme))
    {
        len = sizeof(token->lexeme) - 1;
    }

    memcpy(token->lexeme, lexer->source + lexer->start, len); // copy token text to lexeme
    token->lexeme[len] = '\0';
    return token;
}

// --------------------------------------  KEYWORDS  ----------------------------------------------------

TokenType keywordType(const char *text, int length)
// check if text is a keyword and return correct token type
// default is that the token text is an identifier.
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

    if (length == 4 && strncmp(text, "True", 4) == 0)
        return TOKEN_TRUE;

    if (length == 5 && strncmp(text, "False", 5) == 0)
        return TOKEN_FALSE;

    if (length == 6 && strncmp(text, "return", 6) == 0)
        return TOKEN_RETURN;

    return TOKEN_IDENTIFIER;
}

// --------------------------------------  SCAN IDENTIFIER  ------------------------------------------

void scanIdentifier(Lexer *lexer)
// get the token type from keyword type function and add token accordingly
{
    while (isalnum(peek(lexer)) || peek(lexer) == '_')
    {
        advance(lexer);
    }
    TokenType type =
        keywordType(
            lexer->source + lexer->start,
            lexer->current - lexer->start);

    Token *token = addToken(lexer, type);
    
    if (!token)
        return;

    if(type == TOKEN_TRUE)
    {
        token->literal.boolean = true;
    }
    else if(type == TOKEN_FALSE)
    {
        token->literal.boolean = false;
    }
}

// -------------------------------------  PROCESS STRING ESCAPES  -----------------------------------

static void processStringEscapes(Token *token)
// Interprets \n, \t, \\, \", \' in-place within lexeme, shrinking the string as needed.
// this function also copies the string to the literal.string of the token !
{
    char *src = token->lexeme + 1;                       // skip opening quote
    char *dst = src;
    int len = strlen(token->lexeme);
    char *end = token->lexeme + len - 1;                  // stop before closing quote

    while (src < end)
    {
        if (*src == '\\' && src + 1 < end)
        {
            src++;
            switch (*src)
            {
                case 'n':  *dst++ = '\n'; break;
                case 't':  *dst++ = '\t'; break;
                case '\\': *dst++ = '\\'; break;
                case '"':  *dst++ = '"';  break;
                case '\'': *dst++ = '\''; break;
                default:
                    // unknown escape — keep both characters literally
                    *dst++ = '\\';
                    *dst++ = *src;
                    break;
            }
            src++;
        }
        else
        {
            *dst++ = *src++; // just copy the char to the dst
        }
    }

    *dst = '\0';
    token->literal.string = token->lexeme + 1;
}

// --------------------------------------  SCAN NUMBER  -----------------------------------------

void scanNumber(Lexer *lexer)
// scan numbers, create TOKEN_NUMBER and add it
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
    Token *token = addToken(lexer, TOKEN_NUMBER);
    if (!token)
        return;
    token->literal.number = strtof(token->lexeme, NULL);
}

// --------------------------------------  SCAN ONE TOKEN  ---------------------------------------------

void scanToken(Lexer *lexer)
// scan the current token and process it
{
    char c = advance(lexer);

    switch (c)
    {
        /* Ignore whitespace */
        case ' ':
        case '\r':
        case '\t':
            break;

        /* Newline */
        case '\n':
            addToken(lexer, TOKEN_NEWLINE);
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
        case '#':
        {
            consumeComment(lexer);
            if (!isAtEnd(lexer) || lexer->source[lexer->current - 1] == '\n')
            {
                lexer->start = lexer->current - 1;
                addToken(lexer, TOKEN_NEWLINE);
            }
            break;
        }

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
        {
            char quoteChar = c;
            
            while (!isAtEnd(lexer) && peek(lexer) != quoteChar)
            {
                if (peek(lexer) == '\n')
                {
                    printf("Lexer Error: Newline in string literal.\n");
                    return;
                }
                if (peek(lexer) == '\\' && peekNext(lexer) != '\0')
                {
                    advance(lexer); // skip the backslash so its escaped char isn't mistaken for the closing quote
                }
                advance(lexer);
            }

            if (isAtEnd(lexer))
            {
                printf("Lexer Error: Unterminated string.\n");
                return;
            }
            advance(lexer);
            Token *token = addToken(lexer, TOKEN_STRING);    
            if (!token)
                return;

            break;
        }

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
// run scanToken until the end of the file
{
    while (!isAtEnd(lexer))
    {
        lexer->start = lexer->current;

        scanToken(lexer);
    }
    lexer->start = lexer->current;
    addToken(lexer, TOKEN_EOF);

    for (int i = 0; i < lexer->token_count; i++)
    {
        Token *token = &lexer->tokens[i];
        if (token->type == TOKEN_STRING)
        {
            processStringEscapes(token); // also sets the toke->literal.string
        }
    }
}

// --------------------------------------  DEBUG  ----------------------------------------------------

const char *tokenName(TokenType type)
// return the string name of the provided token type
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

        case TOKEN_TRUE:            return "TRUE";
        case TOKEN_FALSE:           return "FALSE";

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
        case TOKEN_NEWLINE:         return "NEWLINE";
    }

    return "INVALID";
}

const void printTokens(Lexer *lexer)
// print the tokens currently in tokens array
{
    printf("\n========== TOKENS ==========\n\n");

    for (int i = 0; i < lexer->token_count; i++)
    {
        Token *token = &lexer->tokens[i];

        if(token->type == TOKEN_NEWLINE)
        {
            printf("TOKEN_NEWLINE      -> \" \"\n");
        }
        else
        {
            printf("%-18s -> \"%s\"\n",
                tokenName(token->type),
                token->lexeme);
        }
    }
}

// --------------------------------------------  LEX  --------------------------------------------------

void lex(const char *source, Token **out_tokens, int init_capacity)
// endpoint function for parser to access the lexer and get the output as its input
{
    Lexer lexer = {
        .source = source,
        .start = 0,
        .current = 0,
        .tokens = malloc(init_capacity*sizeof(Token)), // initialize the tokens
        .tokens_capacity = init_capacity,
        .token_count = 0
    };

    lexicalAnalyzer(&lexer);

    *out_tokens = lexer.tokens;

    printTokens(&lexer);
}

// --------------------------------------  MAIN  ------------------------------------------------------

int main_lexer(void)
// main function to run the lexer alone, currently renamed to avoid collision with parsers main function
{
    char *source =
        "if x > 5 \n"
        "else \n";

    Lexer lexer =
    {
        .source = source,
        .start = 0,
        .current = 0,
        .tokens = malloc(128*sizeof(Token)), // initialize the tokens
        .tokens_capacity = 128,
        .token_count = 0
    };
    lexicalAnalyzer(&lexer);
    printTokens(&lexer);
    free(lexer.tokens);
    return 0;
}

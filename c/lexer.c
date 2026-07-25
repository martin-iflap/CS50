#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "token.h"

#define MAX_LENGTH 120
#define MAX_TOKENS 128

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

    if (lexer->source[lexer->current + 1] == '\0')
        return '\0';

    return lexer->source[lexer->current + 1];
}

// --------------------------------------  TOKEN CREATION --------------------------------------  

Token *addToken(Lexer *lexer, TokenType type)
{
    if (lexer->token_count >= MAX_TOKENS)
    {
        printf("Too many tokens.\n");
        return NULL;
    }

    Token *token = &lexer->tokens[lexer->token_count++];

    token->type = type;

    memset(&token->literal, 0, sizeof(token->literal));

    int len = lexer->current - lexer->start;
    if (len >= sizeof(token->lexeme))
    {
        len = sizeof(token->lexeme) - 1;
    }

    memcpy(token->lexeme, lexer->source + lexer->start, len);
    token->lexeme[len] = '\0';
    return token;
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
    Token *token = addToken(lexer, TOKEN_NUMBER);
    if (!token)
        return;
    token->literal.number = strtof(token->lexeme, NULL);
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

            int clean_len = strlen(token->lexeme) - 2;
            token->lexeme[clean_len + 1] = '\0';
            token->literal.string = token->lexeme + 1;

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

void printTokens(Lexer *lexer)
{
    printf("\n========== TOKENS ==========\n\n");

    for (int i = 0; i < lexer->token_count; i++)
    {
        Token *token = &lexer->tokens[i];

        printf("%-18s -> \"%s\"\n",
            tokenName(token->type),
            token->lexeme);
    }
}

// --------------------------------------  MAIN  ------------------------------------------------------

int main(void)
{
    char source[MAX_LENGTH] =
        "if x > 5 \n"
        "else \n";

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


// PROGRAM EXAMPLE:
//  x = 5
//  y = 4 * 3 / (2 - 4 + 5) + 2
//  z = x - y
//  if z > x {
//    while z > x {
//      print("z is still greater than x!")
//    }
//  }
//  else {
//    print("x is greater than z!")
//  }

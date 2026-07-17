#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // check if I need all these

#define MAX_LENGTH 120
#define MAX_TOKENS 70


typedef enum
{
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_PRINT,

    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,

    TOKEN_EQUAL,

    TOKEN_LPAREN,
    TOKEN_RPAREN,

    TOKEN_LBRACE,
    TOKEN_RBRACE,

    TOKEN_EOF

} TokenType;


typedef struct
{
    TokenType type;
    char lexeme[64];
} Token;

Token tokens[MAX_TOKENS];


bool isDelimiter(char chr)
// check if the provided character is delimiter
{
    return (chr == ' ' || chr == '+' || chr == '-'
            || chr == '*' || chr == '/' || chr == ','
            || chr == ';' || chr == '%' || chr == '>'
            || chr == '<' || chr == '=' || chr == '('
            || chr == ')' || chr == '[' || chr == ']'
            || chr == '{' || chr == '}');
}

bool isOperator(char chr)
// check if the provided character is operator
{
    return (chr == '+' || chr == '-' || chr == '*'
            || chr == '/' || chr == '>' || chr == '<'
            || chr == '=' || chr == '!');
}

bool isValidIdentifier(char* str)
{
    return (str[0] != '0' && str[0] != '1' && str[0] != '2' // check this function and make sure it's good
            && str[0] != '3' && str[0] != '4'
            && str[0] != '5' && str[0] != '6'
            && str[0] != '7' && str[0] != '8'
            && str[0] != '9' && !isDelimiter(str[0]));
}

bool isKeyword(char* str)
{
    const char* keywords[] = {"if", "else", "while", "print"};
    for (int i = 0;
        i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        if (strcmp(str, keywords[i]) == 0) {
            return true;
        }
    }
    return false;
}

bool isInteger(char* str)
{
    if (str == NULL || *str == '\0') {
        return false;
    }
    int i = 0;
    while (isdigit(str[i])) {
        i++;
    }
    return str[i] == '\0';
}

// trims a substring from a given string's start and end
// position
char* getSubstring(char* str, int start, int end)
{
    int length = strlen(str);
    int subLength = end - start + 1;
    char* subStr
        = (char*)malloc((subLength + 1) * sizeof(char)); // this fucker leaks memory
    strncpy(subStr, str + start, subLength);
    subStr[subLength] = '\0';
    return subStr;
}

int lexicalAnalyzer(char* input)
{
    int left = 0, right = 0;
    int len = strlen(input);

    while (right <= len && left <= right) {
        if (!isDelimiter(input[right]))
            right++;

        if (isDelimiter(input[right]) && left == right) {
            if (isOperator(input[right]))
                printf("Token: Operator, Value: %c\n",
                       input[right]);

            right++;
            left = right;
        }
        else if (isDelimiter(input[right]) && left != right
                 || (right == len && left != right)) {
            char* subStr
                = getSubstring(input, left, right - 1);

            if (isKeyword(subStr))
                printf("Token: Keyword, Value: %s\n",
                       subStr);

            else if (isInteger(subStr))
                printf("Token: Integer, Value: %s\n",
                       subStr);

            else if (isValidIdentifier(subStr)
                     && !isDelimiter(input[right - 1]))
                printf("Token: Identifier, Value: %s\n",
                       subStr);

            else if (!isValidIdentifier(subStr)
                     && !isDelimiter(input[right - 1]))
                printf("Token: Unidentified, Value: %s\n",
                       subStr);
            left = right;
        }
    }
    return 0;
}

// main function
int main()
{
    // Input 01
    char lex_input[MAX_LENGTH] = "int a = b + c";
    printf("For Expression \"%s\":\n", lex_input);
    lexicalAnalyzer(lex_input);
    printf(" \n");
    // Input 02
    char lex_input01[MAX_LENGTH]
        = "if x > 5 {print(x)}";
    printf("For Expression \"%s\":\n", lex_input01);
    lexicalAnalyzer(lex_input01);
    return (0);
}

// gotta make sure this all makes sense and improve it a bit

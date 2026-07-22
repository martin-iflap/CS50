#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "token.h"

#define MAX_STMNTS 100


// ------------------------------------------  EXPRESSION  -----------------------------------------
typedef enum
{
    BinaryExpr,
    UnaryExpr,
    LiteralExpr,
    VariableExpr,
    AssigmentExpr,
    CallExpr,
} ExpressionType;

typedef struct Expression Expression;

struct Expression
{
    ExpressionType type;

    union
    {
        struct
        {
            Token value;
        } literal;

        struct
        {
            Token name;
        } variable;

        struct
        {
            Token op;

            Expression *left;
            Expression *right;
        } binary;

        struct
        {
            Token op;
            Expression *operand;
        } unary;
    };
};

// -----------------------------------------  STATEMENT  -----------------------------------------
typedef struct Program Program;

typedef enum
{
    PrintStmt,
    AssignmentStmt,
    ExpressionStmnt, // gotta make sure we have these sorted and covered right now its a bit of a mess
    IfStmt,
    WhileStmt,
    ReturnStmt,
} StatementType;

typedef struct Statement Statement;

struct Statement
{
    StatementType type;

    union
    {
        struct
        {
            Expression *expression;

        } printStmt;

        struct
        {
            Expression *expression;
        } ExpressionStmt;

        struct
        {
            Token variable;
            Expression *value;
        } assignmentStmt;

        struct
        {
            Expression *condition;
            Program *body;
        } whileIfStmt;
    };
};

// -----------------------------------------  PROGRAM AND PARSER  -----------------------------------------

typedef struct
{
    Statement *statements[MAX_STMNTS];
    int statement_count;
} Program;

typedef struct
{
    Token *tokens;
    int current;
    int token_count; // do i even need this one?
} Parser;

// ------------------------------------------  HELPERS  -----------------------------------------------

bool isAtEnd(Parser *parser)
{
    return parser->tokens[parser->current].type == TOKEN_EOF;
}

Token advance(Parser *parser)
{
    return parser->tokens[parser->current++];
}

Token peek(Parser *parser)
{
    return parser->tokens[parser->current];
}

bool match(Parser *parser, TokenType type)
{
    if (peek(parser).type != type)
        return false;

    advance(parser);
    return true;
}

bool consume(Parser *parser, TokenType expected)
{
    if(peek(parser).type != expected)
    {
        printf("Expected token ...");
        return false;
    }

    advance(parser);
    return true;
}

// -----------------------------------------  PARSE EXPRESSIONS  --------------------------------------------

Expression *parsePrimary(Parser *parser)
{
    Token t = peek(parser);
    Expression *exp = malloc(sizeof(Expression));

    switch (t.type)
    {
    case TOKEN_NUMBER:
        exp->literal.value = ; // get the value here somehow, gotta first fix the stupid ahh lexer and we'll get it
        break;

    case TOKEN_STRING:
    
        break;

    case TOKEN_IDENTIFIER:
    
        break;

    case TOKEN_LBRACE:
        break;
    
    default:
        break;

    return exp;
    }
}

parseUnary(Parser *parser)
{
    Expression *left = parsePrimary(parser);
    Token t = advance(parser);
    if(t.type == TOKEN_MINUS);
    {

    }
}

Expression *parseMultiplication(Parser *parser)
{
    Expression *left = parseUnary(parser);
    Token next_token = advance(parser);

    while(next_token.type == TOKEN_STAR || next_token.type == TOKEN_SLASH || next_token.type == TOKEN_PERCENT)
    {
        Token operator = next_token; // not sure if this is exactly what we want

        next_token = advance(parser);

        Expression *right = parseUnary(parser);

        Expression *exp = malloc(sizeof(Expression)); // do i need to allocate the memory here?
        exp->binary.op = operator;
        exp->binary.left = left;
        exp->binary.right = right;

        left = exp;
    }
    return left;
}

Expression *parseAddition(Parser *parser)
{
    Expression *left = parseMultiplication(parser);
    Token next_token = advance(parser);

    while (next_token.type == TOKEN_PLUS || next_token.type == TOKEN_MINUS)
    {
        Token operator = next_token;

        advance(parser);

        Expression *right = parseMultiplication(parser);

        Expression *exp = malloc(sizeof(Expression)); // same problems as for multiplication
        exp->binary.op = operator;
        exp->binary.left = left;
        exp->binary.right = right;

        left = exp;
    }
    return left;
}

Expression *parseComparison(Parser *parser)
{
    Expression *left = parseAddition(parser);
}

Expression *parseEquality(Parser *parser)
{
    Expression *left = parseComparison(parser);
}

Expression *parseAssignment(Parser *parser)
{
    Expression *left = parseEquality(parser);
}

Expression *parseExpression(Parser *parser)
{
    Expression *exp = parseAssignment(parser);
}

// --------------------------------------------  PARSE BLOCK  --------------------------------------------

Program *parseBlock(Parser *parser)
{
    // basically the same as parse program but stop when you see }
    while(!match(parser, TOKEN_RBRACE))
    {

    }
}

// --------------------------------------------  PARSE STATEMENTS  --------------------------------------------

Statement *parseIfWhileStatement(Parser *parser, Program *program, bool If)
{
    Statement *stmt = malloc(sizeof(Statement));

    if (If)
    {
        stmt->type = IfStmt;
    }
    else
    {
        stmt->type = WhileStmt;
    }
    Expression *e = parseExpression(parser);
    stmt->whileIfStmt.condition = e;

    consume(parser, TOKEN_LBRACE);
    Program *p = parseBlock(parser);
    stmt->whileIfStmt.body = p;

    program->statements[program->statement_count++] = stmt;

    return stmt;
}


Statement *parsePrintStatement(Parser *parser, Program *program)
{
    consume(parser, TOKEN_LBRACKET);

    Statement *stmt = malloc(sizeof(Statement));

    stmt->type = PrintStmt;
    Expression *e = parseExpression(parser);
    stmt->printStmt.expression = e;

    program->statements[program->statement_count++] = stmt;
    return stmt;
}


Statement *parseExpressionStatement(Parser *parser, Program *program)
{
    Statement *stmt = malloc(sizeof(Statement));
    stmt->type = ExpressionStmnt;
    Expression *e = parseExpression(parser);
    stmt->ExpressionStmt.expression = e;

    program->statements[program->statement_count++] = stmt;
    return stmt;
}


Statement *parseStatement(Parser *parser, Program *program)
{
    switch(peek(parser).type)
    {
        case TOKEN_IF:
        {
            return parseIfWhileStatement(parser, program, true);
            break;
        }

        case TOKEN_WHILE:
        {
            return parseIfWhileStatement(parser, program, false);
            break;
        }

        case TOKEN_PRINT:
        {
            return parsePrintStatement(parser, program);
            break;
        }
        default:
        {
            return parseExpressionStatement(parser, program);
            break;
        }        
    }
}


// -------------------------------------------  PARSE PROGRAM  ------------------------------------------

void parseProgram(Parser *parser, Program *program)
{
    while (!isAtEnd(parser))
    {
        parseStatement(parser, program);
    }
}

// ------------------------------------------  MAIN  ----------------------------------------------------

int main()
{
    Parser parser =
    {
        .tokens = , // tokens from lexer, probably make analyze function accessible and also lexer so i can call the function and read values from lexer
        .current = 0,
        .token_count = , // token count from lexer
    };
}

// add better documentation to the code later
// perhaps separate if and while stmts at some point later

#ifndef PARSER_H
#define PARSER_H

#include "../token.h"
#include "../arena/arena_allocator.h"

#define MAX_STMNTS 100


// ------------------------------------------  EXPRESSION  -----------------------------------------
typedef enum
{
    BinaryExpr,
    UnaryExpr,
    LiteralExpr,
    VariableExpr,
    AssignmentExpr,
} ExpressionType;

typedef struct Expression Expression;

typedef enum
{
    LIT_NUMBER,
    LIT_BOOL,
    LIT_STRING,
} LiteralType;

struct Expression
{
    ExpressionType type;

    union
    {
        struct
        {
            LiteralType litType;
            float number;
            bool boolean;
            char *string;
        } literal;

        struct
        {
            char *name;
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
    ExpressionStmnt,
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

        } PrintStmt;

        struct
        {
            Expression *expression;
        } ExpressionStmt;

        struct
        {
            Expression *expression;
        } ReturnStmt;

        struct
        {
            Expression *condition;
            Program *body;
            Program *elseBody;
        } IfStmt;

        struct
        {
            Expression *condition;
            Program *body;
        } WhileStmt;
    };
};

// -----------------------------------------  PROGRAM AND PARSER  -----------------------------------------

struct Program
{
    Statement *statements[MAX_STMNTS];
    int statement_count;
};

typedef struct
{
    Token *tokens;
    int current;
    Arena *arena;
} Parser;

// ----------------------------------------  PARSE PROGRAM  ----------------------------------------------

void parse(const char *source, Program *out_program, Arena **out_arena);

#endif
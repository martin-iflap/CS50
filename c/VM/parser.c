#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "token.h"
#include "lexer.h"
#include "arena/arena_allocator.h"

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

// ------------------------------------------  HELPERS  -----------------------------------------------

static bool isAtEnd(Parser *parser)
// check if we are at the end of the tokens input
{
    return parser->tokens[parser->current].type == TOKEN_EOF;
}

static Token advance(Parser *parser)
// return current token and advance by 1
{
    return parser->tokens[parser->current++];
}

static Token peek(Parser *parser)
// peek at the current token without advancing it
{
    return parser->tokens[parser->current];
}

static bool match(Parser *parser, TokenType type)
// return false if if type doesnt match the token type of current token
// if it matches advance and return true
{
    if (peek(parser).type != type)
        return false;

    advance(parser);
    return true;
}

static bool consume(Parser *parser, TokenType expected)
// consume current token if it matches expected, return true if success else false
{
    if(peek(parser).type != expected)
    {
        printf("Unexpected token, expected %i\n", expected);
        return false;
    }

    advance(parser);
    return true;
}

// -----------------------------------------  PARSE EXPRESSIONS  --------------------------------------------
Expression *parseExpression(Parser *parser);

Expression *parsePrimary(Parser *parser)
// parse primary expressions and return constructed expression
{
    Token t = peek(parser);

    // () expressions            
    if(t.type == TOKEN_LPAREN)
    {
        advance(parser);
        Expression *exp = parseExpression(parser);
        if(!consume(parser, TOKEN_RPAREN))
        {
            printf("Consume failed.\n");
            exit(1);
        }
        return exp;
    }

    Expression *exp = arena_alloc(parser->arena, sizeof(Expression));

    switch (t.type)
    {
        case TOKEN_NUMBER:
            exp->type = LiteralExpr;
            exp->literal.litType = LIT_NUMBER;
            exp->literal.number = t.literal.number;
            advance(parser);
            break;

        case TOKEN_STRING:
            exp->type = LiteralExpr;
            exp->literal.litType = LIT_STRING;
            exp->literal.string = arena_strdup(parser->arena, t.literal.string);
            advance(parser);
            break;

        case TOKEN_IDENTIFIER:
            exp->type = VariableExpr;
            exp->variable.name = arena_strdup(parser->arena, t.lexeme);
            advance(parser);
            break;

        case TOKEN_TRUE:
            exp->type = LiteralExpr;
            exp->literal.litType = LIT_BOOL;
            exp->literal.boolean = true;
            advance(parser);
            break;

        case TOKEN_FALSE:
            exp->type = LiteralExpr;
            exp->literal.litType = LIT_BOOL;
            exp->literal.boolean = false;
            advance(parser);
            break;

        default:
            printf("Unexpected token in expression: %s\n", t.lexeme);
            exit(1);
    }
    return exp;
}

Expression *parseUnary(Parser *parser)
// peek at the current token and if its unary expression parse it
// else return parsePrimary only
{
    Token t = peek(parser);
    Expression *right = NULL;
    if(t.type == TOKEN_MINUS || t.type == TOKEN_BANG || t.type == TOKEN_NOT)
    {
        Token op = t;
        advance(parser);

        Expression *sub_exp = parsePrimary(parser);
        
        Expression *exp = arena_alloc(parser->arena, sizeof(Expression));
        exp->type = UnaryExpr;
        exp->unary.op = op;
        exp->unary.operand = sub_exp;
        right = exp;
    }
    else
    {
        right = parsePrimary(parser);
    }
    return right;
}

Expression *parseMultiplication(Parser *parser)
// parse multiplication
{
    Expression *left = parseUnary(parser);
    Token next_token = peek(parser);

    while(next_token.type == TOKEN_STAR || next_token.type == TOKEN_SLASH || next_token.type == TOKEN_PERCENT)
    {
        Token operator = next_token;
        advance(parser);

        Expression *right = parseUnary(parser);

        Expression *exp = arena_alloc(parser->arena, sizeof(Expression));
        exp->type = BinaryExpr;
        exp->binary.op = operator;
        exp->binary.left = left;
        exp->binary.right = right;

        left = exp;
        next_token = peek(parser);
    }
    return left;
}

Expression *parseAddition(Parser *parser)
// parse addition
{
    Expression *left = parseMultiplication(parser);
    Token next_token = peek(parser);

    while(next_token.type == TOKEN_PLUS || next_token.type == TOKEN_MINUS)
    {
        Token operator = next_token;
        advance(parser);

        Expression *right = parseMultiplication(parser);

        Expression *exp = arena_alloc(parser->arena, sizeof(Expression));
        exp->type = BinaryExpr;
        exp->binary.op = operator;
        exp->binary.left = left;
        exp->binary.right = right;

        left = exp;
        next_token = peek(parser);
    }
    return left;
}

Expression *parseComparison(Parser *parser)
// parse comparison
{
    Expression *left = parseAddition(parser);
    Token next_token = peek(parser);

    while(
        next_token.type == TOKEN_GREATER || next_token.type == TOKEN_LESS ||
        next_token.type == TOKEN_GREATER_EQUAL || next_token.type == TOKEN_LESS_EQUAL
    )
    {
        Token operator = next_token;
        advance(parser);

        Expression *right = parseAddition(parser);

        Expression *exp = arena_alloc(parser->arena, sizeof(Expression));
        exp->type = BinaryExpr;
        exp->binary.op = operator;
        exp->binary.left = left;
        exp->binary.right = right;

        left = exp;
        next_token = peek(parser);
    }
    return left;
}

Expression *parseEquality(Parser *parser)
// parse equality
{
    Expression *left = parseComparison(parser);
    Token next_token = peek(parser);

    while(next_token.type == TOKEN_EQUAL_EQUAL || next_token.type == TOKEN_BANG_EQUAL)
    {
        Token operator = next_token;
        advance(parser);

        Expression *right = parseComparison(parser);

        Expression *exp = arena_alloc(parser->arena, sizeof(Expression));
        exp->type = BinaryExpr;
        exp->binary.op = operator;
        exp->binary.left = left;
        exp->binary.right = right;

        left = exp;
        next_token = peek(parser);
    }
    return left;
}

Expression *parseAndOr(Parser *parser)
// parse and or expressions
{
    Expression *left = parseEquality(parser);
    Token next_token = peek(parser);

    while(next_token.type == TOKEN_AND || next_token.type == TOKEN_OR)
    {
        Token operator = next_token;
        advance(parser);

        Expression *right = parseEquality(parser);

        Expression *exp = arena_alloc(parser->arena, sizeof(Expression));
        exp->type = BinaryExpr;
        exp->binary.op = operator;
        exp->binary.left = left;
        exp->binary.right = right;

        left = exp;
        next_token = peek(parser);
    }
    return left;
}

Expression *parseAssignment(Parser *parser)
// parse assignment expressions
{
    Expression *left = parseAndOr(parser);
    Token token = peek(parser);

    if(token.type == TOKEN_EQUAL) // convert to while perhaps?
    {
        Token operator = token;
        advance(parser);
        Expression *right = parseAndOr(parser);

        Expression *exp = arena_alloc(parser->arena, sizeof(Expression));
        exp->type = AssignmentExpr;
        exp->binary.op = operator;
        exp->binary.left = left;
        exp->binary.right = right;
        
        left = exp;
    }
    return left;
}

Expression *parseExpression(Parser *parser)
// parse expression, calls the whole parse function chain
{
    Expression *exp = parseAssignment(parser);
    return exp;
}

// --------------------------------------------  PARSE BLOCK  --------------------------------------------
Statement *parseStatement(Parser *parser);

Program *parseBlock(Parser *parser)
// basically the same as parse program but also stop when you see }
// build Program *small_program and return it
{
    Program *small_program = arena_alloc(parser->arena, sizeof(Program));
    small_program->statement_count = 0;

    while(!match(parser, TOKEN_RBRACE) && !isAtEnd(parser))
    {
        while (peek(parser).type == TOKEN_NEWLINE)
        {
            if(!consume(parser, TOKEN_NEWLINE))
            {
                printf("Consume failed in parseBlock.\n");
                exit(1);
            }
        }

        if (isAtEnd(parser))
            break;

        Statement *stmt = parseStatement(parser);
        small_program->statements[small_program->statement_count++] = stmt;
    
        if(!match(parser, TOKEN_NEWLINE) && !match(parser, TOKEN_RBRACE))
        {
            printf("Statement not terminated.");
            exit(1);
        }
    
    }
    return small_program;
}

// --------------------------------------------  PARSE STATEMENTS  --------------------------------------------

Statement *parseIfStatement(Parser *parser)
// parse if statement, consume L brace and newlines after the if block
// check if there is else statement after and if yes add it to else body
{
    Statement *stmt = arena_alloc(parser->arena, sizeof(Statement));

    stmt->type = IfStmt;

    stmt->IfStmt.condition = parseExpression(parser);

    if(!consume(parser, TOKEN_LBRACE))
    {
        printf("Consume failed.\n");
        exit(1);
    }
    stmt->IfStmt.body = parseBlock(parser);

    while (peek(parser).type == TOKEN_NEWLINE)
    {
        if(!consume(parser, TOKEN_NEWLINE))
        {
            printf("Consume failed in parseIfStatement.\n");
            exit(1);
        }
    }

    if(match(parser, TOKEN_ELSE))
    {
        if(!consume(parser, TOKEN_LBRACE))
        {
            printf("Consume failed.\n");
            exit(1);
        }
        stmt->IfStmt.elseBody = parseBlock(parser);
    }
    else
    {
        stmt->IfStmt.elseBody = NULL;
    }

    return stmt;
}

Statement *parseWhileStatement(Parser *parser)
// parse while statement
// create Statement with correct type, condition and body
{
    Statement *stmt = arena_alloc(parser->arena, sizeof(Statement));

    stmt->type = WhileStmt;

    stmt->WhileStmt.condition = parseExpression(parser);
    // consume the {
    if(!consume(parser, TOKEN_LBRACE))
    {
        printf("Consume failed.\n");
        exit(1);
    }
    stmt->WhileStmt.body = parseBlock(parser);

    return stmt;
}

Statement *parsePrintStatement(Parser *parser)
// parse print statement and consume the ()
{
    if(!consume(parser, TOKEN_LPAREN))
    {
            printf("Consume failed.\n");
            exit(1);
    }

    Statement *stmt = arena_alloc(parser->arena, sizeof(Statement));

    stmt->type = PrintStmt;
    stmt->PrintStmt.expression = parseExpression(parser);

    if(!consume(parser, TOKEN_RPAREN))
    {
        printf("Consume failed.");
        exit(1);
    }

    return stmt;
}

Statement *parseReturnStatement(Parser *parser)
// parse a return statement and return it
{
    Statement *stmt = arena_alloc(parser->arena, sizeof(Statement));
    stmt->type = ReturnStmt;
    stmt->ReturnStmt.expression = parseExpression(parser);
    
    return stmt;
}

Statement *parseExpressionStatement(Parser *parser)
// parse basic expression statement
{
    Statement *stmt = arena_alloc(parser->arena, sizeof(Statement));
    stmt->type = ExpressionStmnt;
    Expression *e = parseExpression(parser);
    stmt->ExpressionStmt.expression = e;

    return stmt;
}

Statement *parseStatement(Parser *parser)
// peek at the current token type and use statement parsing functions accordingly
// if its keyword advance so we point at a normal token which parseExpression can parse
{
    switch(peek(parser).type)
    {
        case TOKEN_IF:
        {
            advance(parser);
            return parseIfStatement(parser);
            break;
        }

        case TOKEN_WHILE:
        {
            advance(parser);
            return parseWhileStatement(parser);
            break;
        }

        case TOKEN_PRINT:
        {
            advance(parser);
            return parsePrintStatement(parser);
            break;
        }
        case TOKEN_RETURN:
        {
            advance(parser);
            return parseReturnStatement(parser);
            break;
        }
        default:
        {
            return parseExpressionStatement(parser);
            break;
        }        
    }
}

// -------------------------------------------  PARSE PROGRAM  ------------------------------------------

void parseProgram(Parser *parser, Program *program)
// parse statements and add to the program until the end of file
// consume all the new lines but ensure all statements are new line terminated (if not eof)
{
    while (!isAtEnd(parser))
    {
        while (peek(parser).type == TOKEN_NEWLINE)
        {
            if(!consume(parser, TOKEN_NEWLINE))
            {
                printf("Consume failed in parseProgram.\n"); // probably same problem here!
                exit(1);
            }
        }

        if (isAtEnd(parser))
            break;

        Statement *stmt = parseStatement(parser);
        program->statements[program->statement_count++] = stmt;

        if(!match(parser, TOKEN_NEWLINE) && !isAtEnd(parser))
        {
            printf("Statement not terminated.");
            exit(1);
        }
    }
}

// ------------------------------------------  MAIN  ----------------------------------------------------
void printProgram(Program *program, int depth); // fwd decl


int main()
{
    Token tokens[MAX_TOKENS];
    lex(tokens); // still returns token_count but i think i don't need it

    Arena *arena = arena_create(65536);

    Parser parser =
    {
        .tokens = tokens,
        .current = 0,
        .arena = arena,
    };

    Program program = {.statement_count = 0};

    parseProgram(&parser, &program);

    printf("\n========== PROGRAM ==========\n\n");
    printProgram(&program, 0);

    arena_destroy(arena);
}


// TODOs:
// convert assignment to while loop?
// add check to assignment expression that L side is variable expression
// keep an eye on the consume new line logic


// ------------------------------------------  PRINT PROGRAM  ----------------------------------------

static void printIndent(int depth)
// print indent according to the depth
{
    for (int i = 0; i < depth; i++)
        printf("  ");
}

void printExpression(Expression *exp, int depth)
// format and print given expression 
{
    if (!exp)
    {
        printIndent(depth);
        printf("(null expr)\n");
        return;
    }

    printIndent(depth);

    switch (exp->type)
    {
        case LiteralExpr:
            switch (exp->literal.litType)
            {
                case LIT_NUMBER:
                    printf("Literal: %f\n", exp->literal.number);
                    break;
                case LIT_BOOL:
                    printf("Literal: %s\n", exp->literal.boolean ? "True" : "False");
                    break;
                case LIT_STRING:
                    printf("Literal: \"%s\"\n", exp->literal.string);
                    break;
            }
            break;

        case VariableExpr:
            printf("Variable: %s\n", exp->variable.name);
            break;

        case UnaryExpr:
            printf("Unary: %s\n", exp->unary.op.lexeme);
            printExpression(exp->unary.operand, depth + 1);
            break;

        case BinaryExpr:
            printf("Binary: %s\n", exp->binary.op.lexeme);
            printExpression(exp->binary.left, depth + 1);
            printExpression(exp->binary.right, depth + 1);
            break;

        case AssignmentExpr:
            printf("Assign: %s\n", exp->binary.op.lexeme);
            printExpression(exp->binary.left, depth + 1);
            printExpression(exp->binary.right, depth + 1);
            break;
    }
}

void printStatement(Statement *stmt, int depth)
// format and print given statement
{
    if (!stmt)
    {
        printIndent(depth);
        printf("(null stmt)\n");
        return;
    }

    printIndent(depth);

    switch (stmt->type)
    {
        case PrintStmt:
            printf("Print:\n");
            printExpression(stmt->PrintStmt.expression, depth + 1);
            break;

        case ExpressionStmnt:
            printf("ExprStmt:\n");
            printExpression(stmt->ExpressionStmt.expression, depth + 1);
            break;

        case IfStmt:
            printf("If:\n");
            printIndent(depth + 1);
            printf("condition:\n");
            printExpression(stmt->IfStmt.condition, depth + 2);
            printIndent(depth + 1);
            printf("then:\n");
            printProgram(stmt->IfStmt.body, depth + 2);
            if (stmt->IfStmt.elseBody)
            {
                printIndent(depth + 1);
                printf("else:\n");
                printProgram(stmt->IfStmt.elseBody, depth + 2);
            }
            break;

        case WhileStmt:
            printf("While:\n");
            printIndent(depth + 1);
            printf("condition:\n");
            printExpression(stmt->WhileStmt.condition, depth + 2);
            printIndent(depth + 1);
            printf("body:\n");
            printProgram(stmt->WhileStmt.body, depth + 2);
            break;

        case ReturnStmt:
            printf("Return:\n");
            printExpression(stmt->ExpressionStmt.expression, depth + 1);
            break;
    }
}

void printProgram(Program *program, int depth)
// print the whole program array
{
    if (!program)
    {
        printIndent(depth);
        printf("(null program)\n");
        return;
    }

    for (int i = 0; i < program->statement_count; i++)
        printStatement(program->statements[i], depth);
}

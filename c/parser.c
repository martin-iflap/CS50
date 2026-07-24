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
    AssignmentExpr,
} ExpressionType;

typedef struct Expression Expression;

struct Expression
{
    ExpressionType type;

    union
    {
        struct
        {
            float number;
            bool boolean;
            char *string;
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
        printf("Unexpected token, expected %i", expected);
        return false;
    }

    advance(parser);
    return true;
}

// -----------------------------------------  PARSE EXPRESSIONS  --------------------------------------------
Expression *parseExpression(Parser *parser);

Expression *parsePrimary(Parser *parser)
//parse primary expressions
{
    Token t = peek(parser);
    Expression *exp = malloc(sizeof(Expression));

    switch (t.type)
    {
    case TOKEN_NUMBER:
        exp->type = LiteralExpr;
        exp->literal.number = t.literal.number;
        advance(parser);
        break;

    case TOKEN_STRING:
        exp->type = LiteralExpr;
        exp->literal.string = t.literal.string;
        advance(parser);
        break;

    case TOKEN_IDENTIFIER:
        exp->type = VariableExpr;
        exp->variable.name = t; // check if this is the best way to store the identifier, perhaps just copy the lexeme
        advance(parser);
        break;

    case TOKEN_TRUE:
        exp->type = LiteralExpr;
        exp->literal.boolean = true;
        advance(parser);
        break;

    case TOKEN_FALSE:
        exp->type = LiteralExpr;
        exp->literal.boolean = false;
        advance(parser);
        break;

    // () expressions            
    case TOKEN_LPAREN:
        advance(parser);
        exp = parseExpression(parser);
        bool consumed = consume(parser, TOKEN_RPAREN);
        if(!consumed) {
            printf("Consume failed.");
            exit(1);
        }
        break;

    default:
        break;
    }
    return exp;
}

Expression *parseUnary(Parser *parser)
{
    Token t = peek(parser);
    Expression *right = NULL;
    if(t.type == TOKEN_MINUS || t.type == TOKEN_BANG || t.type == TOKEN_NOT) // I have to differentiate the minuses somehow otherwise minus is higher priority than multiplication
    {
        Token op = t;
        advance(parser);

        Expression *sub_exp = parsePrimary(parser);
        
        Expression *exp = malloc(sizeof(Expression));
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
{
    Expression *left = parseUnary(parser);
    Token next_token = peek(parser);

    while(next_token.type == TOKEN_STAR || next_token.type == TOKEN_SLASH || next_token.type == TOKEN_PERCENT)
    {
        Token operator = next_token;

        next_token = advance(parser);

        Expression *right = parseUnary(parser);

        Expression *exp = malloc(sizeof(Expression)); // do i need to allocate the memory here?
        exp->type = BinaryExpr;
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
    Token next_token = peek(parser);

    while(next_token.type == TOKEN_PLUS || next_token.type == TOKEN_MINUS)
    {
        Token operator = next_token;

        advance(parser);

        Expression *right = parseMultiplication(parser);

        Expression *exp = malloc(sizeof(Expression)); // same as for multiplication
        exp->type = BinaryExpr;
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
    Token next_token = peek(parser);

    while(
        next_token.type == TOKEN_GREATER || next_token.type == TOKEN_LESS ||
        next_token.type == TOKEN_GREATER_EQUAL || next_token.type == TOKEN_LESS_EQUAL
    )
    {
        Token operator = next_token;

        advance(parser);

        Expression *right = parseAddition(parser);

        Expression *exp = malloc(sizeof(Expression));
        exp->type = BinaryExpr;
        exp->binary.op = operator;
        exp->binary.left = left;
        exp->binary.right = right;

        left = exp;
    }
    return left;
}

Expression *parseEquality(Parser *parser)
{
    Expression *left = parseComparison(parser);
    Token next_token = peek(parser);

    while(next_token.type == TOKEN_EQUAL_EQUAL || next_token.type == TOKEN_BANG_EQUAL)
    {
        Token operator = next_token;

        advance(parser);

        Expression *right = parseComparison(parser);

        Expression *exp = malloc(sizeof(Expression));
        exp->type = BinaryExpr;
        exp->binary.op = operator;
        exp->binary.left = left;
        exp->binary.right = right;

        left = exp;
    }
    return left;
}

Expression *parseAndOr(Parser *parser)
{
    Expression *left = parseEquality(parser);
    Token next_token = peek(parser);

    while(next_token.type == TOKEN_AND || next_token.type == TOKEN_OR)
    {
        Token operator = next_token;

        advance(parser);

        Expression *right = parseEquality(parser);

        Expression *exp = malloc(sizeof(Expression));
        exp->type = BinaryExpr;
        exp->binary.op = operator;
        exp->binary.left = left;
        exp->binary.right = right;

        left = exp;
    }
    return left;
}

Expression *parseAssignment(Parser *parser)
{
    Expression *left = parseAndOr(parser);
    Token token = peek(parser);

    if(token.type == TOKEN_EQUAL) // convert to while perhaps?
    {
        Token operator = token;
        advance(parser);
        Expression *right = parseAndOr(parser);

        Expression *exp = malloc(sizeof(Expression));
        exp->type = AssignmentExpr;
        exp->binary.op = operator;
        exp->binary.left = left;
        exp->binary.right = right;
        
        left = exp;
    }
    return left;
}

Expression *parseExpression(Parser *parser)
{
    Expression *exp = parseAssignment(parser);
    return exp;
}

// --------------------------------------------  PARSE BLOCK  --------------------------------------------
Statement *parseStatement(Parser *parser, Program *program);

Program *parseBlock(Parser *parser, Program *program)
{
    // basically the same as parse program but stop when you see }
    // gotta create small program here to which i can add the statements and then return the small program ----------- important!!
    while(!match(parser, TOKEN_RBRACE) && !isAtEnd(parser))
    {
        parseStatement(parser, program);
    }
    return program;
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

    bool consumed = consume(parser, TOKEN_LBRACE);
    if(!consumed) {
            printf("Consume failed.");
            exit(1);
    }
    Program *p = parseBlock(parser, program);
    stmt->whileIfStmt.body = p;

    program->statements[program->statement_count++] = stmt;

    return stmt;
}


Statement *parsePrintStatement(Parser *parser, Program *program)
{
    bool consumed = consume(parser, TOKEN_LPAREN);
    if(!consumed) {
            printf("Consume failed.");
            exit(1);
    }

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

    Program program = {.statement_count = 0};

    parseProgram(&parser, &program);
}


// add better documentation to the code later
// perhaps separate if and while stmts and add else to if struct at some point later

// whats the best way to store identifier in an Expression?
// make sure statement types make sense

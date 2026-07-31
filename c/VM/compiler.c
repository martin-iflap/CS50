#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../parser/parser.h"
#include "../vm.h"

#define MAX_CODE 100


// ---------------------------------------  CHUNK AND COMPILER  --------------------------------------------

typedef struct
{
    Instruction code[MAX_CODE];
    int count;
} Chunk;

typedef struct
{
    Statement *program;
    int lenght;
    int current;
} Compiler;

// ----------------------------------------  COMPILE EXPRESSIONS  -----------------------------------------

static void compileExpression(Chunk *chunk, Expression *exp)
{
    switch (exp->type)
    {
        case LiteralExpr:
            emit(chunk, PSH, exp->literal.number); // bool/string need their own handling — see below
            break;

        case VariableExpr:
            emit(chunk, LOAD, resolveVariable(exp->variable.name));
            break;

        case BinaryExpr:
            compileExpression(exp->binary.left, chunk);
            compileExpression(exp->binary.right, chunk);
            emit(chunk, opForToken(exp->binary.op.type), 0);
            break;

        case UnaryExpr:
            compileExpression(exp->unary.operand, chunk);
            emit(chunk, opForToken(exp->unary.op.type), 0);
            break;

        case AssignmentExpr:
            compileExpression(exp->binary.right, chunk);
            emit(chunk, STORE, resolveVariable(exp->binary.left->variable.name));
            break;
    }
}

// --------------------------------------------------  COMPILE STATEMENTS  ------------------------------------------

void compilePrintStmt(Chunk *chunk, Statement *statement)
// add Instruction struct with correct fields to output
{
    Instruction inst = {
        .opcode=PRINT,
        .has_operand=true,
        .operand_type=statement->PrintStmt.expression->type,
        .operand=*statement->PrintStmt.expression->literal.string
    };
    chunk->code[chunk->count++] = inst;
}

void compileIfStmt(Chunk *chunk, Statement *statement)
{
    compileExpression(chunk, statement->WhileStmt.condition);
    Instruction jmp_if_false = {
        .opcode=JMP_IF_FALSE,
        .operand_type=OPERAND_NUMBER,
    };
    int jmp_if_f_index = chunk->count;
    chunk->code[jmp_if_f_index] = jmp_if_false;

    compileProgram(chunk, statement->IfStmt.body);

    Instruction jmp = {
        .opcode=JMP,
        .operand_type=OPERAND_NUMBER
    };
    int jmp_index = chunk->count;
    chunk->code[jmp_index] = jmp;
    
    chunk->code[jmp_if_f_index].has_operand = true;
    chunk->code[jmp_if_f_index].operand.number = (float)(chunk->count - jmp_if_f_index); // fix this
    
    if(statement->IfStmt.elseBody)
    {
        compileProgram(chunk, statement->IfStmt.elseBody); // gotta fwd declare this one
    }
    
    chunk->code[jmp_index].has_operand = true;
    chunk->code[jmp_index].operand.number = (float)(chunk->count - jmp_index);
}

void compileWhileStmt(Chunk *chunk, Statement *statement)
{
    compileExpression(chunk, statement->WhileStmt.condition);
    Instruction jmp_if_false = {
        .opcode=JMP_IF_FALSE,
        .has_operand=true,
        .operand_type=OPERAND_NUMBER,
    };
    int current = chunk->count;
    chunk->code[current] = jmp_if_false;
    compileProgram(chunk, statement->WhileStmt.body); // gotta fwd declare this one
    chunk->code[current].operand.number = (float)(chunk->count - current);
}

void compileReturnStmt(Chunk *chunk, Statement *statement)
{
    // nothing rn
}

void compileStatement(Chunk *chunk, Statement *statement)
{
    switch (statement->type)
    {
        case PrintStmt:
            compilePrintStmt(chunk, statement);
            break;
        case ExpressionStmnt:
            compileExpression(statement->ExpressionStmt.expression, chunk);
            break;
        case IfStmt:
            compileIfStmt(chunk, statement);
            break;
        case WhileStmt:
            compileWhileStmt(chunk, statement);
            break;
        case ReturnStmt:
        default:
            printf("Unknown statement type provided for compilation.");
            exit(1);
    }
}

// ------------------------------------------ COMPILE PROGRAM  -----------------------------------------

void compileProgram(Chunk *chunk, Compiler *compiler)
// compile the statements from the program array one by one
{
    for(int i=0; i<compiler->lenght; i++)
    {
        Statement *stmt = compiler->program[i]; // fix this somehow
        compileStatement(chunk, stmt);
    }
}

// -----------------------------------------------  MAIN  ----------------------------------------------

main()
{
    Chunk chunk = {.count = 0};
    Compiler compiler = {
        .current = 0,
        .lenght = , // get this also from the parser
        .program = , // output from the parser goes here
    };

    compileProgram(&chunk, program);
}


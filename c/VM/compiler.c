#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../parser/parser.h"
#include "../vm.h"

#define MAX_CODE 100
#define MAX_VARS 64

// ---------------------------------------  CHUNK, SYMBOLS, COMPILER  --------------------------------------------

typedef struct
{
    Instruction code[MAX_CODE];
    int count;
} Chunk;

typedef struct
{
    char *names[MAX_VARS];
    int count;
} SymbolTable;

typedef struct
{
    Chunk chunk;
    SymbolTable symbols;
} Compiler;

// -----------------------  FWD DECLS  -----------------------------
static void compileExpression(Compiler *compiler, Expression *exp);
static void compileStatement(Compiler *compiler, Statement *statement);
static void compileProgram(Compiler *compiler, Program *program);

// ----------------------------------------  SYMBOL TABLE  -----------------------------------------

static int resolveVariable(Compiler *compiler, char *name)
// add the name to the symbols if not already added and return the correct index
{
    for (int i = 0; i < compiler->symbols.count; i++)
    {
        if (strcmp(compiler->symbols.names[i], name) == 0)
            return i;
    }

    if (compiler->symbols.count >= MAX_VARS)
    {
        printf("Too many variables.\n");
        exit(1);
    }

    // name is owned by the AST (arena-allocated), so no copy needed here —
    // it will outlive the compiler regardless.
    compiler->symbols.names[compiler->symbols.count] = name;
    return compiler->symbols.count++;
}

// ----------------------------------------  EMIT HELPERS  -----------------------------------------

static void emitSimple(Compiler *compiler, InstructionSet op)
{
    if (compiler->chunk.count >= MAX_CODE)
    {
        printf("Chunk out of space.\n");
        exit(1);
    }
    compiler->chunk.code[compiler->chunk.count++] =
        (Instruction){.opcode = op, .has_operand = false};
}

static void emitNumber(Compiler *compiler, InstructionSet op, float number)
{
    if (compiler->chunk.count >= MAX_CODE)
    {
        printf("Chunk out of space.\n");
        exit(1);
    }
    compiler->chunk.code[compiler->chunk.count++] =
        (Instruction){.opcode = op, .has_operand = true,
                       .operand_type = OPERAND_NUMBER, .operand.number = number};
}

static void emitBool(Compiler *compiler, InstructionSet op, bool boolean)
{
    if (compiler->chunk.count >= MAX_CODE)
    {
        printf("Chunk out of space.\n");
        exit(1);
    }
    compiler->chunk.code[compiler->chunk.count++] =
        (Instruction){.opcode = op, .has_operand = true,
                       .operand_type = OPERAND_BOOLEAN, .operand.boolean = boolean};
}

static void emitString(Compiler *compiler, InstructionSet op, char *string)
{
    if (compiler->chunk.count >= MAX_CODE)
    {
        printf("Chunk out of space.\n");
        exit(1);
    }
    compiler->chunk.code[compiler->chunk.count++] =
        (Instruction){.opcode = op, .has_operand = true,
                       .operand_type = OPERAND_STRING, .operand.string = string};
}

static void emitSlot(Compiler *compiler, InstructionSet op, int slot)
{
    if (compiler->chunk.count >= MAX_CODE)
    {
        printf("Chunk out of space.\n");
        exit(1);
    }
    compiler->chunk.code[compiler->chunk.count++] =
        (Instruction){.opcode = op, .has_operand = true,
                       .operand_type = OPERAND_SLOT, .operand.slot = slot};
}

// Emits a jump with a placeholder offset of 0 and returns the index of that
// instruction, so it can be backpatched once the real target is known.
static int emitJumpPlaceholder(Compiler *compiler, InstructionSet op)
{
    if (compiler->chunk.count >= MAX_CODE)
    {
        printf("Chunk out of space.\n");
        exit(1);
    }
    int index = compiler->chunk.count;
    compiler->chunk.code[compiler->chunk.count++] =
        (Instruction){.opcode = op, .has_operand = true,
                       .operand_type = OPERAND_JUMP, .operand.offset = 0};
    return index;
}

// Patches a previously emitted jump so its offset lands on the chunk's
// current end (i.e. "jump to right here").
static void patchJumpToHere(Compiler *compiler, int jumpIndex)
{
    compiler->chunk.code[jumpIndex].operand.offset = compiler->chunk.count - jumpIndex;
}

// ----------------------------------------  TOKEN -> OPCODE  -----------------------------------------

static InstructionSet opForToken(TokenType type)
{
    switch (type)
    {
        case TOKEN_PLUS:          return ADD;
        case TOKEN_MINUS:         return SBT;
        case TOKEN_STAR:          return MLTP;
        case TOKEN_SLASH:         return DVD;
        case TOKEN_PERCENT:       return MOD;

        case TOKEN_EQUAL_EQUAL:   return EQ;
        case TOKEN_BANG_EQUAL:    return NEQ;
        case TOKEN_LESS:          return LT;
        case TOKEN_GREATER:       return GT;
        case TOKEN_LESS_EQUAL:    return LTE;
        case TOKEN_GREATER_EQUAL: return GTE;

        case TOKEN_AND:           return AND;
        case TOKEN_OR:            return OR;
        case TOKEN_BANG:
        case TOKEN_NOT:           return NOT;

        default:
            printf("No opcode for token type %d\n", type);
            exit(1);
    }
}

// ----------------------------------------  COMPILE EXPRESSIONS  -----------------------------------------

static void compileExpression(Compiler *compiler, Expression *exp)
{
    switch (exp->type)
    {
        case LiteralExpr:
            switch (exp->literal.litType)
            {
                case LIT_NUMBER:
                    emitNumber(compiler, PSH, exp->literal.number);
                    break;
                case LIT_BOOL:
                    emitBool(compiler, PSH, exp->literal.boolean);
                    break;
                case LIT_STRING:
                    emitString(compiler, PSH, exp->literal.string);
                    break;
            }
            break;

        case VariableExpr:
            emitSlot(compiler, LOAD, resolveVariable(compiler, exp->variable.name));
            break;

        case BinaryExpr:
            compileExpression(compiler, exp->binary.left);
            compileExpression(compiler, exp->binary.right);
            emitSimple(compiler, opForToken(exp->binary.op.type));
            break;

        case UnaryExpr:
            compileExpression(compiler, exp->unary.operand);
            if (exp->unary.op.type == TOKEN_MINUS)
                emitSimple(compiler, NEG);
            else
                emitSimple(compiler, opForToken(exp->unary.op.type)); // BANG / NOT
            break;

        case AssignmentExpr:
            compileExpression(compiler, exp->binary.right);
            emitSlot(compiler, STORE, resolveVariable(compiler, exp->binary.left->variable.name));
            break;
    }
}

// --------------------------------------------------  COMPILE STATEMENTS  ------------------------------------------

static void compilePrintStmt(Compiler *compiler, Statement *statement)
{
    compileExpression(compiler, statement->PrintStmt.expression);
    emitSimple(compiler, PRINT);
}

static void compileIfStmt(Compiler *compiler, Statement *statement)
{
    compileExpression(compiler, statement->IfStmt.condition);

    int jumpIfFalseIndex = emitJumpPlaceholder(compiler, JMP_IF_FALSE);

    compileProgram(compiler, statement->IfStmt.body);

    int jumpEndIndex = emitJumpPlaceholder(compiler, JMP);

    // false-condition jump lands here: right after the then-body, before the else-body
    patchJumpToHere(compiler, jumpIfFalseIndex);

    if (statement->IfStmt.elseBody)
    {
        compileProgram(compiler, statement->IfStmt.elseBody);
    }

    // end-of-if jump lands here: after both branches
    patchJumpToHere(compiler, jumpEndIndex);
}

static void compileWhileStmt(Compiler *compiler, Statement *statement)
{
    int loopStart = compiler->chunk.count;

    compileExpression(compiler, statement->WhileStmt.condition);

    int jumpIfFalseIndex = emitJumpPlaceholder(compiler, JMP_IF_FALSE);

    compileProgram(compiler, statement->WhileStmt.body);

    // jump back to re-check the condition
    int jumpBackIndex = emitJumpPlaceholder(compiler, JMP);
    compiler->chunk.code[jumpBackIndex].operand.offset = loopStart - jumpBackIndex;

    // false-condition jump lands here: right after the loop
    patchJumpToHere(compiler, jumpIfFalseIndex);
}

static void compileReturnStmt(Compiler *compiler, Statement *statement)
{
    compileExpression(compiler, statement->ExpressionStmt.expression);
    emitSimple(compiler, RETURN); // to be given real behavior in the VM
}

static void compileStatement(Compiler *compiler, Statement *statement)
{
    switch (statement->type)
    {
        case PrintStmt:
            compilePrintStmt(compiler, statement);
            break;
        case ExpressionStmnt:
            compileExpression(compiler, statement->ExpressionStmt.expression);
            break;
        case IfStmt:
            compileIfStmt(compiler, statement);
            break;
        case WhileStmt:
            compileWhileStmt(compiler, statement);
            break;
        case ReturnStmt:
            compileReturnStmt(compiler, statement);
            break;
        default:
            printf("Unknown statement type provided for compilation.\n");
            exit(1);
    }
}

// ------------------------------------------ COMPILE PROGRAM  -----------------------------------------

static void compileProgram(Compiler *compiler, Program *program)
{
    for (int i = 0; i < program->statement_count; i++)
    {
        compileStatement(compiler, program->statements[i]);
    }
}

// -----------------------------------------------  MAIN  ----------------------------------------------

int main(void)
{
    Compiler compiler = {
        .chunk = {.count = 0},
        .symbols = {.count = 0}
    };

    Program program = {.statement_count = 0}; // placeholder — real output from the parser goes here

    compileProgram(&compiler, &program);

    emitSimple(&compiler, HLT);

    return 0;
}

// fix the vm and probably just go over the file and look for stuff that needs to be added or fixed
// then we need to make sure all the modules are compatible and that the whole thing looks good

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../parser/parser.h"
#include "../vm/vm.h"
#include "compiler.h"

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

    if (compiler->symbols.count >= compiler->symbols.capacity)
    {
        compiler->symbols.capacity *= 2;
        compiler->symbols.names = realloc(compiler->symbols.names, compiler->symbols.capacity * sizeof(char *));
    }

    // name is owned by the AST (arena-allocated), so no copy needed here —
    // it will outlive the compiler regardless.
    compiler->symbols.names[compiler->symbols.count] = name;
    return compiler->symbols.count++;
}

// ----------------------------------------  CHUNK FUNCTIONS  -------------------------------------

void chunk_init(Chunk *chunk)
// initialize chunk
{
    chunk->capacity = 8;
    chunk->code = malloc(chunk->capacity * sizeof(Instruction));
    chunk->count = 0;
}

void chunk_push(Chunk *chunk, Instruction inst)
// push new instruction to the chunk and double the size if needed
{
    if (chunk->count >= chunk->capacity)
    {
        chunk->capacity *= 2;
        chunk->code = realloc(chunk->code, chunk->capacity * sizeof(Instruction));
    }
    chunk->code[chunk->count++] = inst;
}

// ----------------------------------------  EMIT HELPERS  -----------------------------------------

static void emitSimple(Compiler *compiler, InstructionSet op)
{
    chunk_push(&compiler->chunk, (Instruction){.opcode = op, .has_operand = false});
}

static void emitNumber(Compiler *compiler, InstructionSet op, float number)
{
    chunk_push(
        &compiler->chunk,
        (Instruction){.opcode = op, .has_operand = true,
                    .operand_type = OPERAND_NUMBER, .operand.number = number}
    );
}

static void emitBool(Compiler *compiler, InstructionSet op, bool boolean)
{
    chunk_push(
        &compiler->chunk,
        (Instruction){.opcode = op, .has_operand = true,
                    .operand_type = OPERAND_BOOLEAN, .operand.boolean = boolean}
    );
}

static void emitString(Compiler *compiler, InstructionSet op, char *string)
{
    chunk_push(
        &compiler->chunk,
        (Instruction){.opcode = op, .has_operand = true,
                   .operand_type = OPERAND_STRING, .operand.string = strdup(string)} // i have to free this memory in vm later!!!
    );
}

static void emitSlot(Compiler *compiler, InstructionSet op, int slot)
{
    chunk_push(
        &compiler->chunk,
        (Instruction){.opcode = op, .has_operand = true,
                   .operand_type = OPERAND_SLOT, .operand.slot = slot}
    );
}

static int emitJumpPlaceholder(Compiler *compiler, InstructionSet op)
// Emits a jump with a placeholder offset of 0 and returns the index of that
// instruction, so it can be backpatched once the real target is known.
{
    int index = compiler->chunk.count;
    chunk_push(
        &compiler->chunk,
        (Instruction){.opcode = op, .has_operand = true,
                       .operand_type = OPERAND_JUMP, .operand.offset = 0}
    );
    return index;
}

static void patchJumpToHere(Compiler *compiler, int jumpIndex)
// Patches a previously emitted jump so its offset lands on the chunk's current end
{
    compiler->chunk.code[jumpIndex].operand.offset = compiler->chunk.count - jumpIndex - 1; // added -1 to ensure good match in vm
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
    compiler->chunk.code[jumpBackIndex].operand.offset = loopStart - jumpBackIndex - 1; // -1 to adjust it correctly

    // false-condition jump lands here: right after the loop
    patchJumpToHere(compiler, jumpIfFalseIndex);
}

static void compileReturnStmt(Compiler *compiler, Statement *statement)
{
    // compileExpression(compiler, statement->ExpressionStmt.expression); use later if return actually returns real values
    emitSimple(compiler, RETURN);
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
        compileStatement(compiler, &program->statements[i]);
    }
}

// ------------------------------------------  FREE NESTED MEMORY  -----------------------------------

static void freeNestedStatements(Program *program)
// walk the AST and free all the bodies of the statements that have been separately allocated in parseBlock
{
    for (int i = 0; i < program->statement_count; i++)
    {
        Statement *stmt = &program->statements[i];

        switch (stmt->type)
        {
            case IfStmt:
                freeNestedStatements(stmt->IfStmt.body);
                free(stmt->IfStmt.body->statements);

                if (stmt->IfStmt.elseBody)
                {
                    freeNestedStatements(stmt->IfStmt.elseBody);
                    free(stmt->IfStmt.elseBody->statements);
                }
                break;

            case WhileStmt:
                freeNestedStatements(stmt->WhileStmt.body);
                free(stmt->WhileStmt.body->statements);
                break;

            default:
                break; // no nested Program in this statement type
        }
    }
}

// --------------------------------------------  COMPILE  ----------------------------------------------
void printCode(const Instruction *program, int instruction_count); // fwd decl

int compile(const char *source, Instruction **out_code)
{
    Compiler compiler;
    chunk_init(&compiler.chunk);
    compiler.symbols.count = 0;
    compiler.symbols.capacity = 64;
    compiler.symbols.names = malloc(compiler.symbols.capacity * sizeof(char));

    Program program;
    Arena *arena;
    parse(source, &program, &arena);

    compileProgram(&compiler, &program);
    emitSimple(&compiler, HLT);

    freeNestedStatements(&program);
    arena_destroy(arena);
    free(program.statements);

    *out_code = compiler.chunk.code; // hand ownership of the array to the caller

    printCode(*out_code, compiler.chunk.count);

    free(compiler.symbols.names);

    return compiler.chunk.count;
}

// -----------------------------------------------  MAIN  ----------------------------------------------

int compiler_main(void)
{
    // almost the same as compile just make it work on its own
}

// --------------------------------------------  PRINT  ---------------------------------------------

const char *instructionName(InstructionSet opcode)
{
    switch (opcode)
    {
        case PSH: return "PSH";
        case ADD: return "ADD";
        case SBT: return "SBT";
        case POP: return "POP";
        case MLTP: return "MLTP";
        case DVD: return "DVD";
        case SQRT: return "SQRT";
        case SIN: return "SIN";
        case COS: return "COS";
        case MOD: return "MOD";
        case ABS: return "ABS";
        case NEG: return "NEG";

        case EQ: return "EQ";
        case NEQ: return "NEQ";
        case LT: return "LT";
        case GT: return "GT";
        case LTE: return "LTE";
        case GTE: return "GTE";

        case AND: return "AND";
        case OR: return "OR";
        case NOT: return "NOT";

        case JMP: return "JMP";
        case JMP_IF_FALSE: return "JMP_IF_FALSE";

        case STORE: return "STORE";
        case LOAD: return "LOAD";

        case RETURN: return "RETURN";
        case CALL: return "CALL";

        case PRINT: return "PRINT";
        case HLT: return "HLT";
    }

    return "UNKNOWN";
}

void printOperand(const Instruction *instruction)
{
    if (!instruction->has_operand)
        return;

    switch (instruction->operand_type)
    {
        case OPERAND_NUMBER:
            printf("%.2f", instruction->operand.number);
            break;

        case OPERAND_BOOLEAN:
            printf("%s",
                   instruction->operand.boolean ? "true" : "false");
            break;

        case OPERAND_STRING:
            printf("\"%s\"", instruction->operand.string);
            break;

        case OPERAND_SLOT:
            printf("slot %d", instruction->operand.slot);
            break;

        case OPERAND_JUMP:
            printf("%+d", instruction->operand.offset);
            break;

        default:
            printf("<?>");
    }
}

void printCode(const Instruction *program, int instruction_count)
{
    printf("\n========== BYTECODE ==========\n\n");

    for (int i = 0; i < instruction_count; i++)
    {
        printf("%03d: %-15s",
               i,
               instructionName(program[i].opcode));

        if (program[i].has_operand)
        {
            printf(" ");
            printOperand(&program[i]);
        }

        printf("\n");
    }

    printf("\n==============================\n");
}

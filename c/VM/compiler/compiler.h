#ifndef COMPILER_H
#define COMPILER_H

#include "../vm/vm.h"

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

// --------------------------------  COMPILE  --------------------------------

int compile(const char *source, Instruction *out_code);

#endif
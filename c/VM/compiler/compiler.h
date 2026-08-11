#ifndef COMPILER_H
#define COMPILER_H

#include "../vm/vm.h"


// ---------------------------------------  CHUNK, SYMBOLS, COMPILER  --------------------------------------------

typedef struct
{
    Instruction *code;
    int capacity;
    int count;
} Chunk;

typedef struct
{
    char **names;
    int capacity;
    int count;
} SymbolTable;

typedef struct
{
    Chunk chunk;
    SymbolTable symbols;
} Compiler;

// --------------------------------  COMPILE  --------------------------------

int compile(const char *source, Instruction **out_code);

#endif
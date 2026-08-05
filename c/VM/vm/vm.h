#ifndef VM_H
#define VM_H

#include <stdbool.h>

#define STACK_SIZE 256
#define MEM_SIZE 256

// -------------------  Runtime values  ----------------------

typedef enum {
    NUMBER,
    BOOLEAN,
    STRING
} ValueType;

typedef struct {
    ValueType type;

    union {
        float number;
        bool boolean;
        char *string;
    } value;
} Value;

// ---------------------------  Instruction set  -------------------------

typedef enum {
   PSH,     // push
   ADD,     // add 2 top numbers
   SBT,     // subtract top 2 numbers
   POP,     // pop
   MLTP,    // multiply 2 top numbers
   DVD,     // divide 2 top numbers
   SQRT,    // square root of top number
   SIN,     // sin of top number in radians
   COS,     // cos of top number in radians
   MOD,     // modulo
   ABS,     // absolute value
   NEG,     // negate the value

   EQ,      // equal
   NEQ,     // not equal
   LT,      // less than
   GT,      // greater than
   LTE,     // less than or equal
   GTE,     // greater than or equal

   AND,
   OR,
   NOT,

   JMP, // jump by
   JMP_IF_FALSE, // jump by if false

   STORE,
   LOAD,

   RETURN, // maybe add these later but we'll see
   CALL,

   PRINT,
   HLT      // halt

} InstructionSet;

// -------------------------------  Instructions  ---------------------------
typedef enum {
    OPERAND_NONE,
    OPERAND_NUMBER,
    OPERAND_BOOLEAN,
    OPERAND_STRING,
    OPERAND_SLOT,
    OPERAND_JUMP
} OperandType;

typedef struct {
    InstructionSet opcode;
    bool has_operand;
    OperandType operand_type;
    union {
        float number;
        bool boolean;
        char *string;
        int slot; // slot for memory
        int offset; // offset for jumps
    } operand;
} Instruction;

// --------------------------  Virtual Machine  ------------------------------

typedef struct {
    bool running;
    int ip;     // instruction pointer
    int sp;     // stack pointer

    Value stack[STACK_SIZE]; // stack
    Value memory[MEM_SIZE]; // memory array
} VM;


#endif
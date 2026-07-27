#include <stdio.h>
#include <stdbool.h>
#include <math.h>

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

   HLT      // halt

} InstructionSet;

// -------------------------------  Instructions  ---------------------------
// The future compiler will generate these automatically.
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

// ----------------------------  Program  -----------------------------

Instruction program[] = {
    {.opcode = PSH, .has_operand = true, .operand_type = OPERAND_NUMBER, .operand.number = 0},
    {.opcode = COS},
    {.opcode = POP}, // = 1.00

    {.opcode = JMP, .has_operand = true, .operand_type = OPERAND_JUMP, .operand.offset = 6},

    {.opcode = PSH, .has_operand = true, .operand_type = OPERAND_NUMBER, .operand.number = 5},
    { .opcode = PSH, .has_operand = true, .operand_type = OPERAND_NUMBER, .operand.number = 6},
    {.opcode = ADD},
    {.opcode = PSH, .has_operand = true, .operand_type = OPERAND_NUMBER, .operand.number = 4},
    {.opcode = MLTP},
    {.opcode = POP}, // = 44.00

    {.opcode = PSH, .has_operand = true, .operand_type = OPERAND_NUMBER, .operand.number = 67},
    {.opcode = PSH, .has_operand = true, .operand_type = OPERAND_NUMBER, .operand.number = 7},
    {.opcode = SBT},
    {.opcode = PSH, .has_operand = true, .operand_type = OPERAND_NUMBER, .operand.number = 8},
    {.opcode = DVD},
    {.opcode = POP}, // = 7.50

    {.opcode = PSH, .has_operand = true, .operand_type = OPERAND_NUMBER, .operand.number = 1.57f},
    {.opcode = SIN},
    {.opcode = PSH, .has_operand = true, .operand_type = OPERAND_NUMBER, .operand.number = 64},
    {.opcode = MLTP},
    {.opcode = SQRT},
    {.opcode = POP}, // = 8.00

    {.opcode = PSH, .has_operand = true, .operand_type = OPERAND_NUMBER, .operand.number = 67},
    {.opcode = PSH, .has_operand = true, .operand_type = OPERAND_NUMBER, .operand.number = 76},
    {.opcode = GT}, // = false
    {.opcode = NOT},
    {.opcode = PSH, .has_operand = true, .operand_type = OPERAND_NUMBER, .operand.number = 67},
    {.opcode = PSH, .has_operand = true, .operand_type = OPERAND_NUMBER, .operand.number = 76},
    {.opcode = LTE}, // = true
    {.opcode = AND},
    {.opcode = POP}, // = true

    {.opcode = PSH, .has_operand = true, .operand_type = OPERAND_NUMBER, .operand.number = 67},
    {.opcode = PSH, .has_operand = true, .operand_type = OPERAND_NUMBER, .operand.number = 76},
    {.opcode = GT},
    {.opcode = JMP_IF_FALSE, .has_operand = true, .operand_type = OPERAND_JUMP, .operand.offset = 2},
    {.opcode = PSH, .has_operand = true, .operand_type = OPERAND_STRING, .operand.string = "this got skipped"},
    {.opcode = POP},

    {.opcode = HLT}
};

// ---------------------------------  Fetch  --------------------------------

Instruction fetch(VM *vm)
{
    return program[vm->ip];
}

// ------------------------------  Stack push functions  --------------------------

void push_number(VM *vm, float number)
{
    if(vm->sp >= STACK_SIZE - 1)
    {
        printf("Stack overflow.\n");
        vm->running = false;
        return;
    }

    vm->sp++;
    vm->stack[vm->sp].type = NUMBER;
    vm->stack[vm->sp].value.number = number;
}

void push_bool(VM *vm, bool boolean)
{
    if(vm->sp >= STACK_SIZE - 1)
    {
        printf("Stack overflow.\n");
        vm->running = false;
        return;
    }

    vm->sp++;
    vm->stack[vm->sp].type = BOOLEAN;
    vm->stack[vm->sp].value.boolean = boolean;
}

void push_string(VM *vm, char *string)
{
    if(vm->sp >= STACK_SIZE - 1)
    {
        printf("Stack overflow.\n");
        vm->running = false;
        return;
    }

    vm->sp++;
    vm->stack[vm->sp].type = STRING;
    vm->stack[vm->sp].value.string = string;
}

void push_value(VM *vm, Value value)
{
    if(vm->sp >= STACK_SIZE - 1)
        {
            printf("Stack overflow.\n");
            vm->running = false;
            return;
        }
    vm->stack[++vm->sp] = value;
}
// ----------------------------------  Stack pop functions  --------------------------------

Value pop(VM *vm)
// Removes one value from the stack.
{
    if(vm->sp < 0)
    {
        printf("Stack underflow.\n");
        vm->running = false;
        Value empty = {
            .type = NUMBER,
            .value.number = 0
        };
        return empty;
    }

    return vm->stack[vm->sp--];
}

float pop_number(VM *vm)
// Pop specifically a number.
{
    Value value = pop(vm);

    if(value.type != NUMBER)
    {
        printf("VM Error: Expected NUMBER.\n");
        vm->running = false;
        return 0;
    }
    return value.value.number;
}

bool pop_bool(VM *vm)
// Pop specifically a boolean.
{
    Value value = pop(vm);

    if(value.type != BOOLEAN)
    {
        printf("VM Error: Expected BOOLEAN.\n");
        vm->running = false;
        return false;
    }
    return value.value.boolean;
}

char *pop_string(VM *vm)
// Pop specifically a string.
{
    Value value = pop(vm);

    if(value.type != STRING)
    {
        printf("VM Error: Expected STRING.\n");
        vm->running = false;
        return NULL;
    }
    return value.value.string;
}

// ----------------------------  Debug helper  ---------------------------------------

void print_value(Value value)
// Print any Value stored on the stack.
{
    switch(value.type)
    {
        case NUMBER:
            printf("%.2f\n", value.value.number);
            break;

        case BOOLEAN:

            if(value.value.boolean)
                printf("true\n");
            else
                printf("false\n");
            break;

        case STRING:
            printf("%s\n", value.value.string);
            break;
    }
}

// ----------------------------------  Evaluation  -----------------------------
// Execute one instruction.
// The VM receives an instruction and modifies its state.

void eval(Instruction instruction, VM *vm)
{
    switch(instruction.opcode)
    {
        case HLT:
        {
            vm->running = false;
            printf("done\n");
            break;
        }
        case PSH:
        {
            // Push the operand stored inside the instruction.
            switch(instruction.operand_type)
            {
                case OPERAND_NUMBER:
                    push_number(
                        vm,
                        instruction.operand.number
                    );
                    break;

                case OPERAND_BOOLEAN:
                    push_bool(
                        vm,
                        instruction.operand.boolean
                    );
                    break;

                case OPERAND_STRING:
                    push_string(
                        vm,
                        instruction.operand.string
                    );
                    break;
                default:
                {
                    printf("Unsupported operand type %d for PUSH command.", instruction.operand_type);
                    vm->running = false;
                    break;
                }
            }
            break;
        }
        case POP:
        {
            Value value = pop(vm);
            print_value(value);
            break;
        }
        // ------ Arithmetic -------
        case ADD:
        {
            float a = pop_number(vm);
            float b = pop_number(vm);
            float result = b + a;

            push_number(vm, result);
            break;
        }
        case SBT:
        {
            float a = pop_number(vm);
            float b = pop_number(vm);
            float result = b - a;

            push_number(vm, result);
            break;
        }
        case MLTP:
        {
            float a = pop_number(vm);
            float b = pop_number(vm);
            float result = b * a;

            push_number(vm, result);
            break;
        }
        case DVD:
        {
            float a = pop_number(vm);
            if(a == 0)
            {
                printf("Zero Division Error.\n");
                vm->running = false;
                break;
            }

            float b = pop_number(vm);
            float result = b / a;

            push_number(vm, result);
            break;
        }
        // --------------- Math functions ----------------
        case SQRT:
        {
            float number = pop_number(vm);
            float result = sqrt(number);

            push_number(vm, result);
            break;
        }
        case SIN:
        {
            float number = pop_number(vm);
            float result = sin(number);

            push_number(vm, result);
            break;
        }
        case COS:
        {
            float number = pop_number(vm);
            float result = cos(number);

            push_number(vm, result);
            break;
        }
        case ABS:
        {
            float number = pop_number(vm);
            float result = fabs(number);

            push_number(vm, result);
            break;
        }
        case MOD:
        {
            // Currently only supports integer modulo.
            // Later we can add proper integer values.
            float a = pop_number(vm);
            int modulo = instruction.operand.number;
            int result = ((int)a) % modulo;

            push_number(vm, result);
            break;
        }
        // ------------------ Comparisons -----------------
        case EQ:
        {
            float a = pop_number(vm);
            float b = pop_number(vm);

            push_bool(vm, a == b);
            break;
        }
        case NEQ:
        {
            float a = pop_number(vm);
            float b = pop_number(vm);

            push_bool(vm, a != b);
            break;
        }
        case LT:
        {
            float a = pop_number(vm);
            float b = pop_number(vm);

            push_bool(vm, b < a);
            break;
        }
        case GT:
        {
            float a = pop_number(vm);
            float b = pop_number(vm);

            push_bool(vm, b > a);
            break;
        }
        case LTE:
        {
            float a = pop_number(vm);
            float b = pop_number(vm);

            push_bool(vm, b <= a);
            break;
        }
        case GTE:
        {
            float a = pop_number(vm);
            float b = pop_number(vm);

            push_bool(vm, b >= a);
            break;
        }
        // ------------------ Logical operators --------------
        case AND:
        {
            bool a = pop_bool(vm);
            bool b = pop_bool(vm);

            push_bool(vm, a && b);
            break;
        }
        case OR:
        {
            bool a = pop_bool(vm);
            bool b = pop_bool(vm);

            push_bool(vm, a || b);
            break;
        }
        case NOT:
        {
            bool a = pop_bool(vm);

            push_bool(vm, !a);
            break;
        }
        // ------------------ Jumps -------------------
        case JMP:
        {
            if (instruction.operand_type != OPERAND_JUMP)
            {
                printf("Type Error for JMP command operand.");
                vm->running = false;
                break;
            }
            vm->ip = vm->ip + instruction.operand.offset;
            break;
        }
        case JMP_IF_FALSE:
        {
            if (instruction.operand_type != OPERAND_JUMP)
            {
                printf("Type Error for JMP_IF_FALSE command operand.");
                vm->running = false;
                break;
            }
            bool a = pop_bool(vm);
            if (!a)
            {
                vm->ip = vm->ip + instruction.operand.offset;
            }
            break;
        }
        // ------------------ Memory ------------------
        case STORE:
        {
            if (instruction.operand_type != OPERAND_SLOT)
            {
                printf("Type Error for store operand type.");
                vm->running = false;
                break;
            }

            int index = (int)instruction.operand.slot;
            if(index < 0 || index >= MEM_SIZE)
            {
                printf("Memory index out of bounds.\n");
                vm->running = false;
                break;
            }

            vm->memory[index] = pop(vm);
            break;
        }
        case LOAD:
        {
            if (instruction.operand_type != OPERAND_SLOT)
            {
                printf("Type Error for load operand type.");
                vm->running = false;
                break;
            }

            int index = (int)instruction.operand.slot;
            if(index < 0 || index >= MEM_SIZE)
            {
                printf("Memory index out of bounds.\n");
                vm->running = false;
                break;
            }

            Value fetched = vm->memory[index];
            push_value(vm, fetched);
            break;
        }
        default:
        {
            printf(
                "Unknown opcode %d\n",
                instruction.opcode
            );
            vm->running = false;

            break;
        }
    }
}

// -------------------------------------  Main  -------------------------------------

int main()
{
    VM vm = {
        .running = true,
        .ip = 0,
        .sp = -1
    };

    while(vm.running)
    {
        Instruction instruction = fetch(&vm);
        eval(instruction, &vm);
        // Move to next instruction.
        // Later jump instructions will modify this.
        vm.ip++;
    }
    return 0;
}

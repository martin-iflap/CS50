#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#define STACK_SIZE 256


typedef struct {
    bool running;
    int ip;
    int sp;

    float stack[STACK_SIZE];
} VM;

typedef enum {
   PSH, // push
   ADD, // add 2 top numbers
   SBT, // subtract top 2 numbers
   POP, // pop
   MLTP, // multiply 2 top numbers
   DVD, // divide 2 top numbers
   SQRT, // square root of top number
   SIN, // sin of top number in radians
   COS, // cos of top number in radians
   MOD, // take modulo of the number at the top of the stack with the number after MOD operand
   ABS, // absolute value of top number
   HLT // halt
} InstructionSet;

const float program[] = {
    PSH, 0,
    COS,
    POP,
    
    PSH, 5,
    PSH, 6,
    ADD,
    PSH, 4,
    MLTP,
    POP,

    PSH, 67,
    PSH, 7,
    SBT,
    PSH, 8,
    DVD,
    POP,

    PSH, 1.57,
    SIN,
    PSH, 64,
    MLTP,
    SQRT,
    POP,

    HLT
};

int fetch(const VM *vm) {
    // fetch a command from program
    return program[vm->ip];
}

void push(float value, VM *vm) {
    // push a value onto the stack
    if (vm->sp >= STACK_SIZE - 1) {
        printf("Stack overflow.");
        vm->running = false;
    }
    vm->stack[++vm->sp] = value;
}

float pop(VM *vm) {
    // pop and return the top value from the stack
    if (vm->sp < 0) {
        printf("Stack underflow.");
        vm->running = false;
    }
    float popped = vm->stack[vm->sp--];
    return popped;
}

void eval(int instr, VM *arg_vm) {
    // evaluate the instruction
    VM vm = *arg_vm;
    switch (instr) {
        case HLT: {
            vm.running = false;
            printf("done\n");
            break;
        }
        case PSH: {
	        push(program[++vm.ip], &vm);
	        break;
        }
        case POP: {
	        printf("popped %.2f\n", pop(&vm));
	        break;
	    }
	    case ADD: {
	        float a = pop(&vm);
	        float b =pop(&vm);

	        push(a + b, &vm);

	        break;
	    }
        case SBT: {
            float a = pop(&vm);
	        float b = pop(&vm);
            
            push(b - a, &vm);

            break;
        }
        case MLTP: {
            float a = pop(&vm);
            float b = pop(&vm);

            push(a * b, &vm);

            break;
        }
        case DVD: {
            float a = pop(&vm);
            if (a == 0) {
                printf("Zero Division Error.");
                vm.running = false;
                break;
            }
            float b = pop(&vm);

            push((float)b / a, &vm);

            break;
        }
        case SQRT: {
            float num = pop(&vm);
            push(sqrt(num), &vm);

            break;
        }
        case SIN: {
            float num = pop(&vm);
            push(sin(num), &vm);

            break;
        }
        case COS: {
            float num = pop(&vm);
            push(cos(num), &vm);

            break;
        }
        case MOD: {
            int modulo = program[++vm.ip];
            int num = pop(&vm);
            push(num % modulo, &vm);

            break;
        }
        case ABS: {
            float num = pop(&vm);
            push(abs(num), &vm);

            break;
        }
        default: {
            printf("Unknown opcode %d\n", instr);
            vm.running = false;
        }
    }
    *arg_vm = vm;
}

int main() {
    VM vm_1 = {.running = true, .ip=0, .sp=-1};

    while (vm_1.running) {
        eval(fetch(&vm_1), &vm_1);
        vm_1.ip++; // increment the ip every iteration, note: also increments one last time after HLT command.
    }
}

// convert this thing to actuall bytes at some point perhaps

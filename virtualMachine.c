// this is a ssimple virtual machine from http://web.archive.org/web/20200121100942/https://blog.felixangell.com/virtual-machine-in-c/
// to help understand the more complex cInterpreter vm section
#include <stdbool.h>
#include <stdio.h>

#define ip (registers[IP])
#define sp (registers[SP])

// interesting here is when we print out these values they turn into integers. For example psh becomes 0 etc...
typedef enum {
    PSH, // 0
    ADD, // 1
    POP, // 2
    SET, // set register value 3
    HLT // stop the program 4
} InstructionSet;

typedef enum {
    A, B, C, D, E, F, IP, SP, NUM_OF_REGISTERS
} Registers;

const int program[] = {
    PSH, 5,
    PSH, 6,
    ADD,
    POP,
    SET, IP, 0,
    // PSH, 5,
    // PSH, 6,
    // ADD,
    // POP,
    HLT
};

int stack[256];
int registers[NUM_OF_REGISTERS];

int fetch() {
    return program[ip];
}

bool running = true;

void eval(int instr) {
    switch (instr) {
        case HLT: {
            running = false;
            break;
        }
        case PSH: {
            sp++;
            stack[sp] = program[++ip]; // so ++ip means increment ip first then evaluate program[ip]
            printf("Pushed %d on stack sp = %d\n", stack[sp], sp);
            break;
        }
        case ADD: {
            int b = stack[sp--];
            int a = stack[sp--];
            sp++; // reason for this is say our stack looks like [5, 6], we pop off 6 and then 5 sp = -1 unless we increment

            stack[sp] = a + b;
            printf("Added %d onto the stack at sp = %d\n", stack[sp], sp);
            break;
        }
        case POP: {
            int val_popped = stack[sp--];
            printf("Popped %d off the stack! Sp = %d\n", val_popped, sp);
            break;
        }
        case SET: {
            int register_to_set = program[++ip];
            ip++;
            registers[register_to_set] = program[ip];
            printf("Set register %d, ip = %d\n", register_to_set, ip);
            eval(fetch());
            break;
        }
    }
}

int main() {
    sp = -1;
    ip = 0;
    while (running) {
        // int x = fetch();
        // printf("%d\n", x);
        printf("In the loop before eval ip = %d sp = %d\n", ip, sp);
        eval(fetch());
        ip++;
    }
    return 0;
}
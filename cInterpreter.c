#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <fcntl.h>   // For open()
#include <unistd.h>
#include <stdint.h>
#define int long long // preprocessor replaces every keyword int with long long


// instruction set to talk to cpu
enum {
	LEA, IMM, JMP, CALL, JZ, JNZ, ENT, ADJ, LEV, LI, LC, SI, SC, PUSH,
	OR, XOR, AND, EQ, NE, LT, GT, LE, GE, SHL, SHR, ADD, SUB, MUL, DIV, MOD,
	OPEN, READ, CLOS, PRTF, MALC, MSET, MCMP, EXIT
};
// MOV - moves data into registers or the memory, hard to tell type so we break it down into these 5 types

// IMM <num> to put immediate <num> into register AX.
// LC to load a character into AX from a memory address which is stored in AX before execution.
// LI just like LC but dealing with integer instead of character.
// SC to store the character in AX into the memory whose address is stored on the top of the stack.
// SI just like SC but dealing with integer instead of character.

//////////////////////////////////////////////////////////////////////////////// global variables

int token;
char *src, *old_src;
int poolsize;
int line;

int *text, // for the text segment which stores code
	*old_text, // for the text dump segment
	*stack; // for the stack
char *data; // for the data segment for storing initialized data

// registers
int *pc, // program counter stores the memory address of next instruction to run
	*bp, // base pointer, points to elements on the stack
	*sp, // stack pointer, always points to top of stack
	ax, // general register to store result of instruction
	cycle;

//////////////////////////////////////////////////////////////////////////////// global variables
void next() {
	token = *src++; // this is shorthand for *(src++) due to operator precedence
	return;
}

void expression(int level) {
	// do nothing
}

void program() {
	next();
	while(token > 0) {
		// %d= int, %f=float, %c=characters, %s=strings, %%=prints literal %
		printf("token is: %c\n", token);
		next();
	}
}
// pc is program counter stores the memory address of next instruction to run
// quick note on allocation of mem, higher memory addresses are at the start, and lower
// mem addresses are on the bottom. When we push a value we add to the high address then sp--
// when we pop a value we need to go back up so sp++
int eval() {
	int op, *tmp;
	
	while(1) { //next operation code
		op = *pc++; // op equals the
		if (op == IMM) { //IMM <num> to put immediate <num> into register AX
			ax = *pc; // ax is a general register to store result of instruction
			pc++;
		} else if (op == LC) {
			ax = *(char *)ax; //LC to load a character into AX from a memory address which is stored in AX before execution.
		} else if (op == LI) {
			ax = *(int *)ax; //LI just like LC but dealing with integer instead of character.
		} else if (op == SC) {
			// so *(sp++) = *sp the dereferenced value and then sp increments, 
			// the (char*)*(sp++) returns an int that is cast to a string
			// so apparently ax stays the same. The thing I see here though is ax is a mem address not an actual int value
			ax = *(char *)*sp++ = ax; //SC to store the character in AX into the memory whose address is stored on the top of the stack.
		} else if (op == SI) {
			*(int *)*sp++ = ax; //SI just like SC but dealing with integer instead of character.
		} else if (op == PUSH) {
			*--sp = ax; //  push the value in AX onto the stack
		} else if (op == JMP) {
			pc = (int*)*pc; // // jump to the address
		} else if (op == JZ) {
			pc = ax ? pc + 1 : (int *)*pc; // jump if ax is zero
		} else if (op == JNZ) {
			pc = ax ? (int *)*pc : pc + 1; // jump if ax is not zero
		} else if (op == CALL) { // calls a function by changing pc address to functions address
			*--sp = (int)(pc + 1);
			pc = (int *)*pc;
		} else if (op == ENT) { // creates a new call frame that sets size for local variables for a funtion
			*--sp = (int)bp;
			bp = sp;
			sp = sp - *pc++;
		} else if (op == ADJ) { // removes args from stack frame once function call is done
			sp = sp + *pc++;
		} else if (op == LEV) { // restore call frame and PC
			sp = bp;
			bp = (int *)*sp++;
			pc = (int *)*sp++;
		} else if (op == LEA) { // load address for arguments
			ax = (int)(bp + *pc++);
		} else if (op == OR)  ax = *sp++ | ax;
		else if (op == XOR) ax = *sp++ ^ ax;
		else if (op == AND) ax = *sp++ & ax;
		else if (op == EQ)  ax = *sp++ == ax;
		else if (op == NE)  ax = *sp++ != ax;
		else if (op == LT)  ax = *sp++ < ax;
		else if (op == LE)  ax = *sp++ <= ax;
		else if (op == GT)  ax = *sp++ >  ax;
		else if (op == GE)  ax = *sp++ >= ax;
		else if (op == SHL) ax = *sp++ << ax;
		else if (op == SHR) ax = *sp++ >> ax;
		else if (op == ADD) ax = *sp++ + ax;
		else if (op == SUB) ax = *sp++ - ax;
		else if (op == MUL) ax = *sp++ * ax;
		else if (op == DIV) ax = *sp++ / ax;
		else if (op == MOD) ax = *sp++ % ax;
		else if (op == EXIT) { printf("exit(%d)", *sp); return *sp;}
		else if (op == OPEN) { ax = open((char *)sp[1], sp[0]); }
		else if (op == CLOS) { ax = close(*sp);}
		else if (op == READ) { ax = read(sp[2], (char *)sp[1], *sp); }
		else if (op == PRTF) { tmp = sp + pc[1]; ax = printf((char *)tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5], tmp[-6]); }
		else if (op == MALC) { ax = (int)malloc(*sp);}
		else if (op == MSET) { ax = (int)memset((char *)sp[2], sp[1], *sp);}
		else if (op == MCMP) { ax = memcmp((char *)sp[2], (char *)sp[1], *sp);}
		else {
    		printf("unknown instruction:%d\n", op);
    		return -1;
		}
	}
	return 0;
}

int main(int argc, char **argv) {
	int i, fd;

	// because argv[0] = the directory of program name and we just want
	// to make it so when we do argv[0] we get the first argument we
	// actually pass and when we do argc we get the actual argument count
	// that we passed in
	argc--;
	argv++;
	
	poolsize = 250 * 1024; // arbitrary size
	line = 1;
	// fd for file descriptor 0=stdin, 1=stdout , 2=stderr
	// so the 0 actually represents a flag O_RDONLY. I guess the flags also have int values
	// everything writes or reads from a file named /dev/tty
	// inputs write to dev/tty, outputs read from /dev/tty
	// when we write on a key board it writes to dev/tty, and in the standard output stream
	// it writes from dev/tty to standard output
	fd = open(*argv, 0);
	// also open() system call returns a file descriptor which will likely be 3
	// because everytime a process is started it automatically fills 0 1 and 2 for 0=stdin, 1=stdout , 2=stderr
	if (fd < 0) {
		printf("Could not open [%s]\n", *argv);
		return -1;
	}

	// malloc returns null if mem allocation fails
	old_src = malloc(poolsize);
	src = old_src;

	if (old_src == NULL) {
		printf("Could not allocate memory with malloc(%d)\n", poolsize);
		return -1;
	}

	// read(fd, buf, cnt) reads a specified amount of bytes into the buffer
	// returns number of bytes read on success, returns 0 on reaching end of file
	// returns -1 on error or on signal interrupt
	i = read(fd, src, poolsize-1);
	if (i <= 0) {
		printf("read() returned %d\n", i);
		return -1;
	}
	
	// if read returns 0 it means you already read everything and there is nothing more to read
	// we are adding a 0 because src is a char* a.k.a a string and strings needs a null pointer in order
	// for us to know when the string has ended. 'i' = bytes read. However, because
	// our strings are zero based indexed, src[i] = the byte after the last actual char added.
	// so we set it to 0, because that is the same as '\0' in a string. The \ in a string escapes
	// and makes the 0 actually equal to 0.
	src[i] = 0; // add EOF character

	close(fd);

	text = malloc(poolsize);
	if (text == NULL) {
		printf("Could not allocate memory for text segment with malloc(%d)\n", poolsize);
		return -1;
	}
	old_text = text;

	stack = malloc(poolsize);
	if (stack == NULL) {
		printf("Could not allocate memory for stack segment with malloc(%d)\n", poolsize);
		return -1;
	}

	data = malloc(poolsize);
	if (data == NULL) {
		printf("Could not allocate memory for data segment with malloc(%d)\n", poolsize);
		return -1;
	}

	// this basically sets all the bytes to zero for each one, which is different from calloc in that calloc initializes
	// as well, while memset expects the memory to already exist
	memset(text, 0, poolsize);
	memset(data, 0, poolsize);
	memset(stack, 0, poolsize);

	// sp always points to the top of the stack, so stack is size of poolsize
	// it should also be noted that stack goes downward so the top is
	// actually a memory address that is large
	// i am really not understanding how this points to the top of the stack
	// (int*) = a pointer to an int being type casted
	// stack itself is already an int* and then we are typecasting int onto it
	// example uses (int)stack + poolsize but uintptr is better
	// so the point of this is to turn the current address at the stack into
	// an unsigned integer and then add poolsize bytes. If we do not type cast
	// when we add pool size it will add poolsize * the types bytes. So
	// because stack is an integer and an integer is 4 bytes it would add
	// 4000 bytes instead of the 1000 we want to jump up to.
	// Doing (uintptr_t)stack + 1000 will take us to the last byte plus 1
	// so we are actually out of the malloc range here 1 byte beyond what was
	// allocated.
	sp = (int*)((uintptr_t)stack + poolsize);
	bp = sp;
	ax = 0;
	i = 0;
	text[i++] = IMM;
    text[i++] = 10;
    text[i++] = PUSH;
    text[i++] = IMM;
    text[i++] = 20;
    text[i++] = ADD;
    text[i++] = PUSH;
    text[i++] = EXIT;
    pc = text;
	printf("Debugging...");

	// int length = sizeof(text)
	// for (int i = 0; i< text.lenght) {

	// }
	// printf(pc);
	program();

	return eval();
}
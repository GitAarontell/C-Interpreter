#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <fcntl.h>   // For open()
#include <unistd.h>
#include <stdint.h>
#define int long long // preprocessor replaces every keyword int with long long


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
	ax, // general regiter to store result of instruction
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

int eval() {
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
	memset(stack, 0, poolsize);
	memset(data, 0, poolsize);

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
	
	program();

	return eval();
}
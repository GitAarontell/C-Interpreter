SRC_FILES = main.c
CC_FLAGS = -Wall -Wextra -g -std=c11

main: main.c
	echo "Running main program"
	main.exe prompt.c

main.c:
	cc cterp.c -o main

test: main.c
	echo "Running test - should expect exit(30)"
	main.exe hello.c

vm: vm.c
	echo "Running virtual machine"
	vm.exe

vm.c:
	cc virtualMachine.c -o vm

all:
	cc ${SCR_FILES} ${CC_FLAGS} -o mac

clean:
	rm main.exe
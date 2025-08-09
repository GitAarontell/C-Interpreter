main: main.c
	echo "Running main program"
	main.exe prompt.c

main.c:
	cc cterp.c -o main

clean:
	rm main.exe
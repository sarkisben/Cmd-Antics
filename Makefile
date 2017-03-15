all: cmd-antics

cmd-antics: cmd-antics.c
	gcc -o cmd cmd-antics.c
clean:
	rm -f cmd-antics.o

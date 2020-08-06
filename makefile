CC=clang
CFLAGS = -g #-Wall -Werror

all: example

example: tree.o
	@echo "\n--Example--\n"
	@./example

tree.o:
	$(CC) $(CFLAGS) -c tree.c
	$(CC) $(CFLAGS) example.c -c
	$(CC) $(CFLAGS) tree.o example.o -o example

clean:
	rm -rf tree.dSYM
	rm tree.o
	rm example.o
	rm example
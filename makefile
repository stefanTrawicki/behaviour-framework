CC=clang
CFLAGS = -g #-Wall -Werror

all: tree.o

debug: tree.o
	@echo "\n--Debug--\n"
	@lldb example

example: tree.o
	@echo "\n--Example--\n"
	@./example

log: tree.o
	@echo "\n--Example--\n"
	@./example log.txt

tree.o:
	$(CC) $(CFLAGS) tree.c -c
	$(CC) $(CFLAGS) example.c -c
	$(CC) $(CFLAGS) tree.o example.o -o example

clean:
	rm -rf tree.dSYM
	rm -f tree.o
	rm -f tree
	rm -f example.o
	rm -f example
	rm -f log.txt
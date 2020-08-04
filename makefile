CC=clang
CFLAGS = -g -Wall

all: bt

tree.o: tree.h tree.c
	$(CC) $(CLFAGS) -c tree.c

bt: tree.c example.c
	$(CC) $(CFLAGS) -o bt example.c tree.c

example:
	@echo "Running Example..."
	./bt

clean:
	rm -rf bt.dSYM bt
CC=clang
DB=lldb
CFLAGS=-g -Wall

clean:
	rm -f *.so *.o *.out implementation
	rm -rf *.dSYM

compile: clean behaviour-library/behaviour.c
	$(CC) $(CFLAGS) behaviour-library/behaviour.c -shared -fPIC -o libbehaviour.so
	cp libbehaviour.so /usr/local/lib
	cp behaviour.h /usr/local/include

example: clean compile implementation.c
	$(CC) $(CFLAGS) implementation.c -o implementation -L. -lbehaviour
	#

debug: clean compile implementation.c
	$(CC) $(CFLAGS) implementation.c -o implementation -L. -lbehaviour
	$(DB) implementation
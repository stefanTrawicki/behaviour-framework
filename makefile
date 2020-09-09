CC=clang
CFLAGS=-Wall -g
BINS=libbehaviourtree.so

clean:
	rm -f *.o *.txt example
	rm -rf *.dSYM
	rm -rf *.so

libbehaviourtree.so: behaviourtree.c behaviourtree.h
	$(CC) $(CFLAGS) -fPIC -shared -o $@ behaviourtree.c -lpointerlist -lpointerstack
	cp libbehaviourtree.so /usr/local/lib
	cp behaviourtree.h /usr/local/include

install: libbehaviourtree.so

example: install
	$(CC) $(CFLAGS) -o example example.c -lbehaviourtree -lpointerlist
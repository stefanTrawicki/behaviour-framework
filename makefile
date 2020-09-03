CC=clang
CFLAGS=-Wall -g
BINS=libbehaviourtree.dylib

clean:
	rm -f *.o *.txt example
	rm -rf *.dSYM
	rm -rf *.dylib

libbehaviourtree.dylib: behaviourtree.c behaviourtree.h
	$(CC) $(CFLAGS) -fPIC -shared -o $@ behaviourtree.c -L. -lpointerlist
	rm -f /usr/local/lib/libbehaviourtree.dylib
	cp libbehaviourtree.dylib /usr/local/lib
	rm -f /usr/local/include/behaviourtree.h
	cp behaviourtree.h /usr/local/include

install: libbehaviourtree.dylib clean

example: install
	$(CC) $(CFLAGS) -o example example.c -lbehaviourtree
	@echo "Example\n"
	./example "log.txt"
CC=clang
CFLAGS=-Wall -g
BINS=libmylist.dylib libbehaviourtree.dylib example

clean:
	rm -f *.o
	rm -rf *.dSYM
	rm -rf *.dylib

libmylist.dylib: libs/mylist.c libs/mylist.h
	$(CC) $(CFLAGS) -fPIC -shared -o $@ libs/mylist.c
	cp libmylist.dylib /usr/local/lib
	cp libs/mylist.h /usr/local/include

libbehaviourtree.dylib: libs/behaviourtree.c libs/behaviourtree.h
	$(CC) $(CFLAGS) -fPIC -shared -o $@ libs/behaviourtree.c -L. -lmylist
	cp libbehaviourtree.dylib /usr/local/lib
	cp libs/behaviourtree.h /usr/local/include

install: libbehaviourtree.dylib libmylist.dylib clean

example: install
	$(CC) $(CFLAGS) -o example example.c -lbehaviourtree
	@echo "Example\n"
	./example "log.txt"
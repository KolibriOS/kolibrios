all:
	fasm network.asm
	ar -csr libnetwork.a network.o
clean:
	rm -f *.o *.a
install:
	cp libnetwork.a ../../bin/lib

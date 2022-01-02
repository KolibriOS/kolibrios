all:
	fasm buf2d.asm buf2d.o
	ar -csr libbuf2d.a buf2d.o

clean:
	rm -f *.a
	rm -f *.o

install:
	mv *.a ../../bin/lib

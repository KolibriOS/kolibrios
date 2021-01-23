all:
	fasm loadlibimg.asm
	ar -csr libimg.a loadlibimg.o

install:	
	mv libimg.a ../../bin/lib/libimg.a

clean:
	rm -f *.o *.a

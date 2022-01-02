all:
	fasm boxlib.asm
	ar -crs libbox.a boxlib.o

clean:
	rm -f *.a
	rm -f *.o

install:
	mv libbox.a ../../bin/lib

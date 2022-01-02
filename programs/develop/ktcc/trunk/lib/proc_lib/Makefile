all:
	fasm loadproclib.asm
	ar -csr libdialog.a loadproclib.o
install:
	mv libdialog.a ../../bin/lib
clean:
	rm -f *.o *.a

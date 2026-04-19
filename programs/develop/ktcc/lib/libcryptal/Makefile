CC= ../../bin/kos32-tcc
LIBNAME=libcryptal.a
AR=ar

CFLAGS= -c -I ../../libc.obj/include -I.

all:
	$(CC) $(CFLAGS) *.c 
	$(AR) -rcs $(LIBNAME) *.o
clean:
	rm -f *.o $(LIBNAME)
install:
	mv $(LIBNAME) ../../bin/lib

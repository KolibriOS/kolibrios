all:
	../../bin/kos32-tcc -c gb.c -I ../../libc/include
	ar -rcs libgb.a gb.o
clean:
	rm -f *.o
	rm -f *.a
install:
	cp libgb.a ../../bin/lib

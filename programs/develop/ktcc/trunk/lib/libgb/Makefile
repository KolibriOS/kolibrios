# gcc not kos32-gcc
all:
	gcc -c -O2 -nostdinc -nostdlib -m32 -march=i686 -fomit-frame-pointer -fno-builtin -fno-builtin-printf gb.c
	ar -rcs libgb.a gb.o
clean:
	rm -f *.o
	rm -f *.a
install:
	cp libgb.a ../../bin/lib

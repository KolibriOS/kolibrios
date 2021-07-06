ifndef GCC
	GCC=kos32-gcc
endif

KPACK=kpack
FASM=fasm 

CFLAGS =  -c -nostdinc -I../include -DGNUC -D_BUILD_LIBC -fno-common -Os -fno-builtin -fno-leading-underscore -fno-pie

SRC=libc.c
LIB=libc.obj

all:
	$(MAKE) -C ../linuxtools/src
	mkdir -p exports
	../linuxtools/ExportGen symbols.txt exports/exports.c
	$(FASM) crt/crt0.asm ../../bin/lib/crt0.o
	$(GCC) $(CFLAGS) $(SRC) -o $(LIB)
	$(KPACK) $(LIB)
	../linuxtools/LoaderGen symbols.txt ../loader
	../linuxtools/LoaderBuild ../loader
	$(MAKE) -C libtcc
	rm -rf exports
install:
	cp -f libc.obj ~/.kex/root/RD/1/LIB

clean:
	rm ../../bin/lib/libc.obj.a ../../bin/lib/libtcc.a

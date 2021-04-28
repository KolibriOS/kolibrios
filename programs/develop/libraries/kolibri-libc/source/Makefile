ifndef GCC
	GCC=kos32-gcc
endif

KPACK=kpack
FASM=fasm 

CFLAGS =  -c -nostdinc -I../include -DGNUC -D_KOLIBRI_LIBC_OBJ -Os -fno-common -fno-builtin -fno-leading-underscore -fno-pie

SRC=libc.c
LIB=../lib/libc.obj

all:
	$(MAKE) -C ../linuxtools/src
	mkdir -p exports ../lib
	../linuxtools/ExportGen symbols.txt exports/exports.c
	$(FASM) crt/crt0.asm ../lib/crt0.o
	$(GCC) $(CFLAGS) $(SRC) -o $(LIB)
	$(KPACK) $(LIB)
	../linuxtools/LoaderGen symbols.txt ../loader
	../linuxtools/LoaderBuild ../loader
	rm -rf exports
install:
	cp -f ../lib/libc.obj ~/.kex/root/RD/1/LIB

clean:
	rm ../lib/libc.obj

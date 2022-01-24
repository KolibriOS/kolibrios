KTCC=kos32-tcc

FASM=fasm

OBJ= memcpy.o  memmove.o  memset.o  libtcc1.o

all: $(OBJ)
	ar -crs ../../../bin/lib/libtcc.a $(OBJ)
	rm -f *.o

%.o : %.c
	$(KTCC) -c $< -o $@ 

%.o : %.asm
	$(FASM) $< $@
	

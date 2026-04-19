KTCC=kos32-tcc
FASM=fasm

OBJ= memcpy.o  memmove.o  memset.o  libtcc1.o

all: $(OBJ)
	ar -crs ../../../bin/lib/libtcc1.a $(OBJ)
	rm -f *.o

%.o : %.c
	$(KTCC) -c $< -o $@ 

%.o : %.asm
	$(FASM) $< $@


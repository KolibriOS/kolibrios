KTCC_DIR=../../../../ktcc/trunk

KTCC=$(KTCC_DIR)/bin/kos32-tcc
FASM=fasm

OBJ= memcpy.o  memmove.o  memset.o  libtcc1.o

all: $(OBJ)
	ar -crs ../../lib/libtcc.a $(OBJ)
	rm -f *.o

%.o : %.c
	$(KTCC) -c $< -o $@ 

%.o : %.asm
	$(FASM) $< $@
	

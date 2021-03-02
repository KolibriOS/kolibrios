FASM = fasm
MKLIB = ../linuxtools/mklib

SRCS=$(wildcard *.asm)
OBJS=$(SRCS:.asm=.o)

%.o : %.asm
	$(FASM) $<

all: asm_gen $(OBJS)
	ar -rsc ../lib/libc.obj.a *.o

asm_gen:
	cat symbols.txt | $(MKLIB) 

clean:
	rm -f *.o

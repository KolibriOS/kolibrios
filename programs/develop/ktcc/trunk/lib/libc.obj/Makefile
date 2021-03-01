FASM = fasm
MKLIB = ./tools/mklib

SRCS=$(wildcard *.asm)
OBJS=$(SRCS:.asm=.o)

%.o : %.asm
	$(FASM) $<

all: asm_gen $(OBJS)
	ar -rsc ../../bin/lib/libc.obj.a *.o

asm_gen:
	cat symbols.txt | $(MKLIB) 

clean:
	rm ../../bin/lib/libc.obj.a
	rm -f *.o

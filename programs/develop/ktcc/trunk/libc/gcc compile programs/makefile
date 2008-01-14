OUTFILE = test.kex
INPUT   = test.c
OUTPUT  = test.o
OBJS    = start.o test.o 


.SUFFIXES: .asm .o

$(OUTFILE) : $(OBJS)
	fasm start.asm start.o
	gcc -c $(INPUT) -nostdinc -m32 -I/home/andrew/kolibri/develop/include/
	ld -nostdlib -T kolibri.ld -n -m elf_i386 -L/home/andrew/kolibri/develop/lib/ -o $(OUTFILE) $(OBJS) -lck
	objcopy $(OUTFILE) -O binary

.asm.o:
	fasm $*.asm

.c.o:
	gcc -c $*.c

clean :
	del *.o

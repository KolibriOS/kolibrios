all:
	fasm start.asm start.o
	kos32-gcc -c -fno-builtin shell.c
	kos32-gcc -c -fno-builtin system/kolibri.c
	kos32-gcc -c -fno-builtin system/stdlib.c
	kos32-gcc -c -fno-builtin system/string.c
	kos32-gcc -c -fno-builtin system/ctype.c
	kos32-ld -nostdlib -T kolibri.ld -o shell start.o kolibri.o stdlib.o string.o ctype.o shell.o 
	kos32-objcopy shell -O binary

clean: 
	rm -f *.o
	rm -f bin/rus/shell
	rm -f bin/eng/shell

install:
	cp shell $(DIR)

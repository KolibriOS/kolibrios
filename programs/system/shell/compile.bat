del shell.kex
del shell.o
fasm start.asm start.o
gcc -c shell.c
ld -nostdlib -T kolibri.ld -o shell.kex start.o kolibri.o stdlib.o string.o shell.o
objcopy shell.kex -O binary
pause
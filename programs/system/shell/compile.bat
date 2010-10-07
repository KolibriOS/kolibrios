fasm start.asm start.o
gcc -c shell.c
gcc -c system/kolibri.c
gcc -c system/stdlib.c
gcc -c system/string.c
ld -nostdlib -T kolibri.ld -o shell.kex start.o kolibri.o stdlib.o string.o shell.o
objcopy shell.kex -O binary
kpack shell.kex
pause
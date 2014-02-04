@echo off
fasm start.asm start.o
gcc -c -fno-builtin foxhunt.c
gcc -c -fno-builtin system/kolibri.c
gcc -c -fno-builtin system/stdlib.c
gcc -c -fno-builtin system/string.c
gcc -c -fno-builtin system/ctype.c
ld -nostdlib -T kolibri.ld -o FOXHUNT start.o kolibri.o stdlib.o string.o ctype.o foxhunt.o
objcopy FOXHUNT -O binary
erase start.o foxhunt.o kolibri.o stdlib.o string.o ctype.o
kpack FOXHUNT
pause

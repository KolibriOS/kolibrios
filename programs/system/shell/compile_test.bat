@echo off
fasm start.asm start.o
gcc -c -fno-builtin test.c
gcc -c -fno-builtin system/kolibri.c
gcc -c -fno-builtin system/stdlib.c
gcc -c -fno-builtin system/string.c
gcc -c -fno-builtin system/ctype.c
ld -nostdlib -T kolibri.ld -o test start.o kolibri.o stdlib.o string.o ctype.o test.o
objcopy test -O binary
erase start.o kolibri.o stdlib.o string.o ctype.o test.o
kpack test
copy test bin\eng\
move test bin\rus\
pause

@echo off
erase shell.kex lang.h
echo #define LANG_RUS 1 > lang.h
fasm start.asm start.o
gcc -c shell.c
gcc -c system/kolibri.c
gcc -c system/stdlib.c
gcc -c system/string.c
ld -nostdlib -T kolibri.ld -o shell.kex start.o kolibri.o stdlib.o string.o shell.o
objcopy shell.kex -O binary
erase lang.h start.o shell.o kolibri.o stdlib.o string.o
kpack shell.kex
move shell.kex bin\rus\
copy locale\rus\.shell bin\rus\
pause
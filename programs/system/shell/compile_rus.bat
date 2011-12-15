@echo off
erase lang.h
echo #define LANG_RUS 1 > lang.h
fasm start.asm start.o
gcc -c shell.c
gcc -c system/kolibri.c
gcc -c system/stdlib.c
gcc -c system/string.c
gcc -c system/ctype.c
ld -nostdlib -T kolibri.ld -o shell start.o kolibri.o stdlib.o string.o ctype.o shell.o
objcopy shell -O binary
erase lang.h start.o shell.o kolibri.o stdlib.o string.o
kpack shell
move shell bin\rus\
copy locale\rus\.shell bin\rus\
pause
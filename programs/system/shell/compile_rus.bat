@echo off
echo #define LANG_RUS 1 > lang.h
fasm start.asm start.o
gcc -c -fno-builtin shell.c
gcc -c -fno-builtin system/kolibri.c
gcc -c -fno-builtin system/stdlib.c
gcc -c -fno-builtin system/string.c
gcc -c -fno-builtin system/ctype.c
ld -nostdlib -T kolibri.ld -o shell start.o kolibri.o stdlib.o string.o ctype.o shell.o
objcopy shell -O binary
erase lang.h start.o shell.o kolibri.o stdlib.o string.o ctype.o
kpack shell
move shell bin\rus\
copy locale\rus\.shell bin\rus\
pause

del *.o
fasm asm_code.asm start.o
echo #define LANG_RUS 1 > lang.h
gcc -c z80/z80.c
gcc -c system/kolibri.c
gcc -c system/stdlib.c
gcc -c system/string.c
gcc -c e80.c
ld -nostdlib -T kolibri.ld -o e80.kex start.o kolibri.o stdlib.o string.o z80.o e80.o 
objcopy e80.kex -O binary
kpack e80.kex
pause
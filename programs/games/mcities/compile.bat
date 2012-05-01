del *.o
fasm asm_code.asm asm_code.o
gcc -c mcities.c
gcc -c system/kolibri.c
gcc -c system/stdlib.c
gcc -c system/string.c
gcc -c system/ctype.c
ld -nostdlib -T kolibri.ld -o mcities asm_code.o kolibri.o stdlib.o string.o ctype.o mcities.o
objcopy mcities -O binary
kpack mcities
del *.o
pause
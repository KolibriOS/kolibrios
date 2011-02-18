del *.o
del example.kex
fasm asm_code.asm start.o
gcc -c example.c
gcc -c system/kolibri.c
gcc -c system/stdlib.c
gcc -c system/string.c
ld -nostdlib -T kolibri.ld -o example.kex start.o kolibri.o stdlib.o string.o example.o
objcopy example.kex -O binary
kpack example.kex
del *.o
pause
del *.o
fasm asm_code.asm asm_code.o
gcc -c c_code.c
gcc -c system/kolibri.c
gcc -c system/stdlib.c
gcc -c system/string.c
gcc -c system/gblib.c
ld -nostdlib -T kolibri.ld -o piton asm_code.o kolibri.o stdlib.o string.o gblib.o c_code.o
objcopy piton -O binary
kpack piton
del *.o
pause
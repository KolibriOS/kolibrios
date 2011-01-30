del *.o
fasm asm_code.asm asm_code.o
gcc -c c_code.c
gcc -c system/kolibri.c
gcc -c system/stdlib.c
gcc -c system/string.c
ld -nostdlib -T kolibri.ld -o console15 asm_code.o kolibri.o stdlib.o string.o c_code.o
objcopy console15 -O binary
kpack console15
del *.o
pause
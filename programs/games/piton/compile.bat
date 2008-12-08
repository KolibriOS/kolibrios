fasm asm_code.asm
gcc -c c_code.c
ld -nostdlib -T kolibri.ld -o piton.kex asm_code.obj kolibri.o stdlib.o string.o gblib.o c_code.o
objcopy piton.kex -O binary
pause
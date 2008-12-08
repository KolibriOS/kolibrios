fasm asm_code.asm
gcc -c c_code.c
ld -nostdlib -T kolibri.ld -o donkey.kex asm_code.obj kolibri.o stdlib.o string.o gblib.o c_code.o
objcopy donkey.kex -O binary
pause
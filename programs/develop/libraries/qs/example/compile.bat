gcc -c example.c
ld -nostdlib -T kolibri.ld -o example.kex start.o kolibri.o stdlib.o string.o example.o
objcopy example.kex -O binary
pause
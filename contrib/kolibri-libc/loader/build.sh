#!/bin/bash
set -e

MKLIB=../linuxtools/mklib
AR=ar
FASM=fasm

set -e

echo "Generate ASM files..."
cat symbols.txt | $MKLIB

echo "Compile ASM files..."
for asm_file in $(find *.asm) 
do
    $FASM $asm_file  > /dev/null
done

echo "Create libc.obj.a library..."
ar -rsc ../lib/libc.obj.a *.o
mv __lib__.asm __lib__.asm.bak
rm *.o *.asm
mv __lib__.asm.bak __lib__.asm
echo "Done!"

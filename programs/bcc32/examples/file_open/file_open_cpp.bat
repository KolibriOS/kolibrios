Set NAME=file_open
Set BCC_DIR=..\..\..\bcc32
kos32-bcc -S -v- -R- -6 -a4 -O2 -Og -Oi -Ov -OS -k- -D__KOLIBRI__ -I..\..\..\bcc32\include %NAME%.cpp

echo STACKSIZE equ 8192> kos_make.inc
echo HEAPSIZE equ 0>> kos_make.inc
echo include "%BCC_DIR%\include\kos_start.inc">> kos_make.inc
echo include "%BCC_DIR%\include\kos_func.inc">> kos_make.inc
echo include "%BCC_DIR%\include\kos_heap.inc">> kos_make.inc

echo include "kos_make.inc" > f_%NAME%.asm
t2fasm < %NAME%.asm >> f_%NAME%.asm
fasm f_%NAME%.asm %NAME%.kex
if exist %NAME%.kex kpack %NAME%.kex
if exist %NAME%.kex del kos_make.inc
pause

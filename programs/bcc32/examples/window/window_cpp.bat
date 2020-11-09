kos32-bcc -S -v- -R- -6 -a4 -O2 -Og -Oi -Ov -OS -k- -D__KOLIBRI__ -I..\..\..\bcc32\include window.cpp

echo STACKSIZE equ 102400> kos_make.inc
echo HEAPSIZE equ 102400>> kos_make.inc
echo include "..\..\..\bcc32\include\kos_start.inc">> kos_make.inc
echo include "..\..\..\bcc32\include\kos_func.inc">> kos_make.inc
echo include "..\..\..\bcc32\include\kos_heap.inc">> kos_make.inc

echo include "kos_make.inc" > f_window.asm
t2fasm < window.asm >> f_window.asm
fasm f_window.asm window.kex
if exist window.kex kpack window.kex
if exist window.kex del kos_make.inc
pause

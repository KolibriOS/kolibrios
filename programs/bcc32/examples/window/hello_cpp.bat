kos32-bcc -S -v- -R- -6 -a4 -O2 -Og -Oi -Ov -OS -k- -D__KOLIBRI__ -Iinclude hello.cpp

echo STACKSIZE equ 102400 \n HEAPSIZE equ 102400 \n include "..\..\..\proc32.inc" \ninclude "$(INCLUDE)/kos_start.inc" \n include "$(INCLUDE)/kos_func.inc" \n include "$(INCLUDE)/kos_heap.inc" > kos_make.inc

echo include "kos_make.inc" > f_hello.asm
t2fasm < hello.asm >> f_hello.asm
fasm f_hello.asm hello.kex
kpack hello.kex
pause

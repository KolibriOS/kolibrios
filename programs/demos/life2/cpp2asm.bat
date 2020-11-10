kos32-bcc -S -v- -R- -6 -a4 -O2 -Og -Oi -Ov -OS -k- -D__KOLIBRI__ -Iinclude life2.cpp
echo include "kos_make.inc" > f_life2.asm
t2fasm < life2.asm >> f_life2.asm

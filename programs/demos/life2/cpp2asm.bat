bcc32 -S -v- -R- -6 -a4 -O2 -Og -Oi -Ov -OS -k- -D__MENUET__ -Iinclude life2.cpp
echo include "me_make.inc" > f_life2.asm
t2fasm < life2.asm >> f_life2.asm
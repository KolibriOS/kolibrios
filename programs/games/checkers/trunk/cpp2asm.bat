bcc32 -S -v- -R- -6 -a4 -O2 -Og -Oi -Ov -OS -k- -x- -D__MENUET__ -Iinclude checkers.cpp
echo include "me_make.inc" > f_checkers.asm
t2fasm < checkers.asm >> f_checkers.asm
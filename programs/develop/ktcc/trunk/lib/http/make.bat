fasm __lib__.asm
fasm get.asm
fasm head.asm
fasm post.asm
fasm receive.asm
fasm send.asm
kos32-ar -ru libhttp.a *.o
del *.o
pause
fasm __lib__.asm
fasm InputBox.asm
kos32-ar -ru libinputbox.a *.o
del *.o
pause
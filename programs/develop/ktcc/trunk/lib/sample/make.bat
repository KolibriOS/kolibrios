fasm __lib__.asm
fasm sample_symbol.asm
kos32-ar -ru libsample.a *.o
del *.o
pause
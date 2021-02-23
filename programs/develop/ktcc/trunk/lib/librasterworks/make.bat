fasm __lib__.asm
fasm mb_create.asm 
fasm mb_reinit.asm  
fasm mb_setfunctions.asm
kos32-ar -ru librasterworks.a *.o
del *.o
pause

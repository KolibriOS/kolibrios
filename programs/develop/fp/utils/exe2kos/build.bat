@echo off
fpc -Twin32 exe2kos.pp
del *.o
del *.ppu
move exe2kos.exe ..\..\bin
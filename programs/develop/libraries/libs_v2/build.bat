set FILE_NAME=%1
set DIR_LIB=C:\Programs\KlbrInWin\LIB

del "%FILE_NAME%.obj"
gcc -w --target=alpha-coff -nostdlib -I"./include" --prefix=/opt/cross-gcc -c -o "%FILE_NAME%.obj" "%FILE_NAME%.c"
kpack %FILE_NAME%.obj
@echo off
copy "%FILE_NAME%.obj" "%DIR_LIB%\%FILE_NAME%.obj">Nul

pause
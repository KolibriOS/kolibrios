@echo off
rem Compiler:  NASM
rem Target OS: KolibriOS

rem NASM Path:
SET UF_NASM=\nasm

if not exist "%UF_NASM%\nasmw.exe" goto Err1
"%UF_NASM%\nasmw" -fbin -t -O5 -i..\ufmodlib\src\ mini.asm
goto TheEnd

:Err1
echo Couldn't find nasmw.exe in %UF_NASM%\

:TheEnd
pause
@echo on
cls

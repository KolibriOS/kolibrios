@echo off
rem Make the uFMOD examples.
rem Compiler:  FASM
rem Target OS: KolibriOS

rem FASM Path:
SET UF_FASM=\fasm

if not exist "%UF_FASM%\fasm.exe" goto Err1
"%UF_FASM%\fasm" mini.asm mini
"%UF_FASM%\fasm" jmp2pat.asm jmp2pat
goto TheEnd

:Err1
echo Couldn't find fasm.exe in %UF_FASM%\

:TheEnd
pause
@echo on
cls

@echo off
rem Make the uFMOD libraries in COFF object format
rem Target OS: KolibriOS

rem *** CONFIG START
rem *** Check the Readme docs for a complete reference
rem *** on configuring the following options

rem Pathes:
SET UF_MASM=\masm32\bin
SET UF_NASM=\nasm
SET UF_FASM=\fasm

rem Select compiler: MASM, NASM or FASM
SET UF_ASM=FASM

rem Select mixing rate: 22050, 44100 or 48000 (22.05 KHz, 44.1 KHz or 48 KHz)
SET UF_FREQ=48000

rem Set volume ramping mode (interpolation): NONE, WEAK or STRONG
SET UF_RAMP=STRONG

rem Set build mode: NORMAL, UNSAFE or AC97SND
SET UF_MODE=NORMAL
rem *** CONFIG END

if %UF_ASM%==MASM goto MASM
if %UF_ASM%==NASM goto NASM
if %UF_ASM%==FASM goto FASM
echo %UF_ASM% not supported
goto TheEnd

:MASM
if not exist "%UF_MASM%\ml.exe" goto Err1
"%UF_MASM%\ml" /c /coff /nologo /Df%UF_FREQ% /D%UF_RAMP% /D%UF_MODE% /Fo ufmod.obj src\masm.asm
goto TheEnd

:NASM
if not exist "%UF_NASM%\nasmw.exe" goto Err2
"%UF_NASM%\nasmw" -O4 -t -fwin32 -dNODEBUG -df%UF_FREQ% -d%UF_RAMP% -d%UF_MODE% -isrc\ -oufmod.obj src\nasm.asm
goto TheEnd

:FASM
if not exist "%UF_FASM%\fasm.exe" goto Err3
echo UF_FREQ  equ %UF_FREQ%  >tmp.asm
echo UF_RAMP  equ %UF_RAMP% >>tmp.asm
echo UF_MODE  equ %UF_MODE% >>tmp.asm
echo DEBUG    equ 0         >>tmp.asm
echo NOLINKER equ 0         >>tmp.asm
echo include 'src\eff.inc'  >>tmp.asm
echo include 'src\fasm.asm' >>tmp.asm
"%UF_FASM%\fasm" tmp.asm ufmod.obj
del tmp.asm
goto TheEnd

:Err1
echo Couldn't find ml.exe    in %UF_MASM%\
goto TheEnd
:Err2
echo Couldn't find nasmw.exe in %UF_NASM%\
goto TheEnd
:Err3
echo Couldn't find fasm.exe  in %UF_FASM%\

:TheEnd
pause
@echo on
cls

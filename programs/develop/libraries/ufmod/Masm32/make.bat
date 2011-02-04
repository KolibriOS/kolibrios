@echo off
rem Set compiler location:
SET MASM32=\masm32
SET UF_FASM=\fasm

if not exist "%MASM32%\bin\ml.exe" goto Err1
if not exist "%UF_FASM%\fasm.exe"  goto Err2
"%MASM32%\bin\ml" /c /coff mini.asm
"%MASM32%\bin\link" /DRIVER /SUBSYSTEM:NATIVE /BASE:-0x10000 /ALIGN:0x10000 /MERGE:.data=.text -ignore:4078 mini.obj ufmod.obj
del mini.obj
echo virtual at 0                     >tmp.asm
echo file 'mini.exe':3Ch,4           >>tmp.asm
echo load pehea dword from 0         >>tmp.asm
echo file 'mini.exe':pehea+0F8h,28h >>tmp.asm
echo load physofs dword from 4+14h   >>tmp.asm
echo load mem dword from 4+8         >>tmp.asm
echo file 'mini.exe':physofs+16,4   >>tmp.asm
echo load sz dword from $-4          >>tmp.asm
echo end virtual                     >>tmp.asm
echo file 'mini.exe':physofs,sz      >>tmp.asm
echo store dword mem at 14h          >>tmp.asm
"%UF_FASM%\fasm" tmp.asm mini
del mini.exe
del tmp.asm

goto TheEnd
:Err1
echo Couldn't find ml.exe   in %MASM32%\bin
goto TheEnd
:Err2
echo Couldn't find fasm.exe in %UF_FASM%\

:TheEnd
pause
cls

@echo off
set NASM="nasm\nasm.exe"
rem set GCC="D:\MinGW_32_bit\bin\gcc.exe"
set GCC="C:\MinGW\msys\1.0\home\autobuild\tools\win32\bin\kos32-gcc.exe"

rem %NASM% -f coff "lib\msvcrt.dll.asm" -o "..\lib\msvcrt.dll"
rem strip --strip-debug "../lib/msvcrt.dll"

rem %GCC% -fno-ident -nostdlib -fno-builtin -c -std=gnu99 -o ..\lib\msvcrt.dll lib\msvcrt.dll.c
%GCC% -fno-ident -fno-builtin -O0 -c -o ..\lib\msvcrt.dll lib\msvcrt.dll.c

%NASM% -f coff "lib/kernel32.dll.asm" -o "../lib/kernel32.dll"
strip --strip-debug "../lib/kernel32.dll"

%NASM% -f bin "kwine.asm" -o "../kwine"

if %errorlevel% == 0 (
    echo compiled succesfully.
    ubuntu1804 run "mcopy -D o -i ../kolibri.img ../lib/msvcrt.dll ::kwine/lib/msvcrt.dll"
    ubuntu1804 run "mcopy -D o -i ../kolibri.img ../lib/kernel32.dll ::kwine/lib/kernel32.dll"
    ubuntu1804 run "mcopy -D o -i ../kolibri.img ../kwine ::kwine/kwine"

    qemu-system-x86_64 -fda ../kolibri.img -m 256
) else (
    echo compilation failed.
)

pause
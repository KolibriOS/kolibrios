@echo off
set NASM="nasm\nasm.exe"
%NASM% -f coff "lib\msvcrt.dll.asm" -o "..\lib\msvcrt.dll"
strip --strip-debug "../lib/msvcrt.dll"

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
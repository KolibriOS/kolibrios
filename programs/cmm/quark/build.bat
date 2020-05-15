@ECHO OFF

C-- "quark.c"
del "quark"
rename "quark.com" "quark"
del warning.txt

if exist quark (
    "C:\Program Files (x86)\WinImage\winimage.exe" "D:\Kolibri\Desktop\kolibri.img" /H /Q /I quark
    d:
    cd "D:\Kolibri\Infrastructure\QEMU"
    call "z_kos.bat"
) else (
    pause
)

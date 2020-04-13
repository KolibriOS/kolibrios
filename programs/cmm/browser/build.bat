@ECHO OFF

call "compile_ru.bat"

if exist WebView (
    "C:\Program Files (x86)\WinImage\winimage.exe" "D:\Kolibri\Desktop\kolibri.img" /H /Q /I WebView
    d:
    cd "D:\Kolibri\Infrastructure\QEMU"
    call "z_kos.bat"
) else (
    pause
)

@ECHO OFF

call "compile_ru.bat"

if exist WebView (
    "C:\Program Files (x86)\WinImage\winimage.exe" "C:\Users\leency\Desktop\kolibri.img" /H /Q /I WebView
    d:
    cd "D:\Kolibri\Infrastructure\QEMU"
    call "C:\Users\leency\Desktop\qemu.bat"
) else (
    pause
)

@ECHO OFF

@c-- /D=LANG_RUS WebView.c
@del WebView
@rename WebView.com WebView
@del warning.txt

if not exist WebView ( @pause )

if exist WebView (
    "C:\Program Files (x86)\WinImage\winimage.exe" "C:\Users\leency\Desktop\kolibri.img" /H /Q /I WebView
    d:
    cd "D:\Kolibri\Infrastructure\QEMU"
    call "C:\Users\leency\Desktop\qemu.bat"
) else (
    pause
)

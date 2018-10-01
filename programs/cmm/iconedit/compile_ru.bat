@del lang.h--
@echo #define LANG_RUS 1 >lang.h--
@del iconedit
cls

@c-- iconedit.c
@rename *.com *.
@del warning.txt
@del lang.h--

if exist iconedit (
    "C:\Program Files (x86)\WinImage\winimage.exe" "C:\Users\Leency\Desktop\kolibri.img" /H /Q /I iconedit
    d:
    cd "D:\Kolibri\Infrastructure\QEMU"
    call "z_kos.bat"
) else (
    @pause
)

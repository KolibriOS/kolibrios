@del lang.h--
@echo #define LANG_RUS 1 >lang.h--
@del iconedit
cls

@c-- iconedit.c
@rename *.com *.
@del warning.txt
@del lang.h--

if exist iconedit (
    "C:\Program Files\WinImage\winimage.exe" "D:\Soft\Kolibri\QEMU\kolibri.img" /H /Q /I iconedit
    d:
    cd "D:\Soft\Kolibri\QEMU"
    call "D:\Soft\Kolibri\QEMU\qemu-kos-img.bat"
) else (
    @pause
)

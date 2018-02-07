@del iconedit
@c-- iconedit.c
@rename *.com *.
@del warning.txt


if exist iconedit (
    "C:\Program Files (x86)\WinImage\winimage.exe" "C:\Users\Leency\Desktop\kolibri.img" /H /Q /I iconedit
    cd D:\Kolibri\Infrastructure\QEMU
    call "D:\Kolibri\Infrastructure\QEMU\qemu-kos-img.bat"
) else (
    @pause
)

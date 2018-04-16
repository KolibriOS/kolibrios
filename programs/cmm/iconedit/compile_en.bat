@del iconedit
@c-- iconedit.c
@rename *.com *.
@del warning.txt


if exist iconedit (
    "C:\Program Files\WinImage\winimage.exe" "D:\Soft\Kolibri\QEMU\kolibri.img" /H /Q /I iconedit
    d:
    cd "D:\Soft\Kolibri\QEMU"
    call "D:\Soft\Kolibri\QEMU\qemu-kos-img.bat"
) else (
    @pause
)

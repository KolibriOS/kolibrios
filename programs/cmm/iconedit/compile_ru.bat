@c-- /D=LANG_ENG iconedit.c
@del iconedit
@rename *.com *.
@del warning.txt

if exist iconedit (
    "C:\Program Files (x86)\WinImage\winimage.exe" "D:\Kolibri\Desktop\kolibri.img" /H /Q /I iconedit
    d:
    cd "D:\Kolibri\Infrastructure\QEMU"
    call "z_kos.bat"
) else (
    @pause
)

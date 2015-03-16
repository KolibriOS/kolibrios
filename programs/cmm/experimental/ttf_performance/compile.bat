C-- ttf_performance.c
@del ttf_performance
@rename ttf_performance.com ttf_performance
kpack ttf_performance
@del warning.txt
@pause
@rem ====== Automatically add binnary to kolibri.img and then run QEMU =====
"C:\Program Files (x86)\WinImage\WINIMAGE.exe" D:\Kolibri\work\QEMU\kolibri.img  /I /H/Q C:\Users\lee\Desktop\CMM\ttf_performance\ttf_performance
@cd /d D:\Kolibri\work\QEMU
D:\Kolibri\work\QEMU\qemu-kos-img.bat
C-- "example original font.c"
@del "example original font"
@rename "example original font.com" "example original font"
kpack "example original"
@del warning.txt
@pause
@rem ====== Automatically add binnary to kolibri.img and then run QEMU =====
@rem"C:\Program Files (x86)\WinImage\WINIMAGE.exe" D:\Kolibri\work\QEMU\kolibri.img  /I /H/Q C:\Users\Leency\Dropbox\CMM\example\example
@rem @cd /d C:\Work\QEMU
@rem C:\Work\QEMU\kolibri_qumu.bat
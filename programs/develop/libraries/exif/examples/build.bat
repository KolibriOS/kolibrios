if not exist bin mkdir bin
@fasm.exe -m 16384 viev_exif.asm bin\viev_exif.kex
@kpack bin\viev_exif.kex
if not exist bin\toolbar.png @copy toolbar.png bin\toolbar.png
if not exist bin\font8x9.bmp @copy ..\..\..\..\fs\kfar\trunk\font8x9.bmp bin\font8x9.bmp
@fasm.exe -m 16384 ..\trunk\exif.asm bin\exif.obj
@kpack bin\exif.obj
pause
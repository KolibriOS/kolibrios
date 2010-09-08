if not exist bin mkdir bin
if not exist bin\font8x9.bmp   @copy ..\..\..\fs\kfar\trunk\font8x9.bmp bin\font8x9.bmp
@copy *.png bin\*.png
@copy *.ini bin\*.ini
@fasm.exe -m 16384 nu_pogod.asm bin\nu_pogod.kex
@kpack bin\nu_pogod.kex
if not exist bin\buf2d.obj @fasm.exe -m 16384 ..\..\..\develop\libraries\buf2d\trunk\buf2d.asm bin\buf2d.obj
@kpack bin\buf2d.obj
pause
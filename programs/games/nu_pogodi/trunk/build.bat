if not exist bin mkdir bin
if not exist bin\font8x9.bmp   @copy ..\..\..\fs\kfar\trunk\font8x9.bmp bin\font8x9.bmp
if not exist bin\chi.png    @copy chi.png bin\chi.png
if not exist bin\curici.png @copy curici.png bin\curici.png
if not exist bin\eggs.png   @copy eggs.png bin\eggs.png
if not exist bin\wolf.png   @copy wolf.png bin\wolf.png
@fasm.exe -m 16384 nu_pogod.asm bin\nu_pogod.kex
@kpack bin\nu_pogod.kex
if not exist bin\buf2d.obj @fasm.exe -m 16384 ..\..\..\develop\libraries\buf2d\trunk\buf2d.asm bin\buf2d.obj
@kpack bin\buf2d.obj
pause
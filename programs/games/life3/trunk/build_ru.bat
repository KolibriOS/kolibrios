if not exist bin mkdir bin
@copy *.png bin\*.png
@fasm.exe -m 16384 life3.asm bin\life3.kex
@kpack bin\life3.kex
if not exist bin\buf2d.obj @fasm.exe -m 16384 ..\..\..\develop\libraries\buf2d\trunk\buf2d.asm bin\buf2d.obj
@kpack bin\buf2d.obj
pause

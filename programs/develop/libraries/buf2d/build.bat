if not exist bin mkdir bin
@fasm.exe -m 16384 buf2d.asm bin\buf2d.obj
@kpack bin\buf2d.obj
pause
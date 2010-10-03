if not exist bin mkdir bin
@fasm.exe  -m 16384 box_lib.asm bin\box_lib.obj
@kpack bin\box_lib.obj
pause
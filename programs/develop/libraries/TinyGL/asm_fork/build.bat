if not exist bin mkdir bin
@fasm.exe -m 16384 tinygl.asm bin\tinygl.obj
@kpack bin\tinygl.obj
pause
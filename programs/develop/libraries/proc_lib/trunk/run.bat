if not exist bin mkdir bin
@fasm.exe  -m 16384 proc_lib.asm bin\proc_lib.obj
@kpack bin\proc_lib.obj
pause
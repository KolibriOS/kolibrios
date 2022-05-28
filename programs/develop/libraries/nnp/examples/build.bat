if not exist bin mkdir bin
@fasm.exe ..\nnp.asm bin\nnp.obj

@fasm.exe -m 16384 nnp_points.asm bin\nnp_points.kex
@kpack nnp_points.kex
pause
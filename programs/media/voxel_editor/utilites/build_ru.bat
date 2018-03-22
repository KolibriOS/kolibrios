if not exist bin mkdir bin
if not exist bin\buf2d.obj @fasm.exe -m 16384 ..\..\..\develop\libraries\TinyGL\asm_fork\tinygl.asm bin\tinygl.obj
@kpack bin\tinygl.obj
@fasm.exe -m 16384 vox_creator.asm bin\vox_creator.kex
@kpack bin\vox_creator.kex
@fasm.exe -m 16384 vox_mover.asm bin\vox_mover.kex
@kpack bin\vox_mover.kex
@fasm.exe -m 16384 vox_tgl.asm bin\vox_tgl.kex
@kpack bin\vox_tgl.kex
pause
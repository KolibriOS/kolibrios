if not exist bin mkdir bin
if not exist bin\toolbar.png @copy toolbar.png bin\toolbar.png
if not exist bin\toolbar_m.png @copy toolbar_m.png bin\toolbar_m.png
if not exist bin\toolbar_t.png @copy toolbar_t.png bin\toolbar_t.png
if not exist bin\buf2d.obj @fasm.exe -m 16384 ..\..\..\develop\libraries\TinyGL\asm_fork\tinygl.asm bin\tinygl.obj
@kpack bin\tinygl.obj
@fasm.exe -m 16384 vox_creator.asm bin\vox_creator.kex
@kpack bin\vox_creator.kex
@fasm.exe -m 16384 vox_mover.asm bin\vox_mover.kex
@kpack bin\vox_mover.kex
@fasm.exe -m 16384 vox_tgl.asm bin\vox_tgl.kex
@kpack bin\vox_tgl.kex
pause
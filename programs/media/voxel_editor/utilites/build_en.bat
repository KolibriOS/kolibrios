if not exist bin mkdir bin
@erase lang.inc
@echo lang fix en >lang.inc
@fasm.exe -m 16384 vox_creator.asm bin\vox_creator.kex
@kpack bin\vox_creator.kex
@fasm.exe -m 16384 vox_mover.asm bin\vox_mover.kex
@kpack bin\vox_mover.kex
@fasm.exe -m 16384 vox_tgl.asm bin\vox_tgl.kex
@kpack bin\vox_tgl.kex
pause
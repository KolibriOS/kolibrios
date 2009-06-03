REM compiling examples
if not exist bin mkdir bin
@fasm.exe  -m 16384 ctrldemo.asm bin\ctrldemo.kex
@kpack bin\ctrldemo.kex
@copy cnv_bmp.obj bin\cnv_bmp.obj
@copy reload_16x16_8b.bmp bin\reload_16x16_8b.bmp

@fasm.exe  -m 16384 editbox_ex.asm bin\editbox_ex.kex
@kpack bin\editbox_ex.kex

pause
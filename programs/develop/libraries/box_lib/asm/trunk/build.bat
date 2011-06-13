REM compiling examples
if not exist bin mkdir bin

@fasm.exe  -m 16384 ctrldemo.asm bin\ctrldemo.kex
@kpack bin\ctrldemo.kex

@copy reload_16x16_8b.png bin\reload_16x16_8b.png

@fasm.exe  -m 16384 editbox_ex.asm bin\editbox_ex.kex
@kpack bin\editbox_ex.kex

@pause
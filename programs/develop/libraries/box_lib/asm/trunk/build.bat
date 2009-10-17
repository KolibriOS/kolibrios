REM compiling examples
if not exist bin mkdir bin

@fasm.exe  -m 16384 ctrldemo.asm bin\ctrldemo.kex
@kpack bin\ctrldemo.kex

@fasm.exe  -m 16384 OpenDial.asm bin\OpenDial.kex
@kpack bin\OpenDial.kex

@copy reload_16x16_8b.png bin\reload_16x16_8b.png
@copy cnv_png.obj bin\cnv_png.obj
@copy icons.ini bin\icons.ini
@copy z_icons.png bin\z_icons.png

@fasm.exe  -m 16384 editbox_ex.asm bin\editbox_ex.kex
@kpack bin\editbox_ex.kex

pause
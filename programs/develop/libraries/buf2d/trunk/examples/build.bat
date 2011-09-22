if not exist bin mkdir bin
@fasm.exe -m 16384 ..\buf2d.asm bin\buf2d.obj
@kpack bin\buf2d.obj

if not exist bin\font8x9.bmp @copy ..\..\..\..\..\fs\kfar\trunk\font8x9.bmp bin\font8x9.bmp
@copy foto.jpg bin\foto.jpg
@copy img0.png bin\img0.png
@copy img1.png bin\img1.png

@fasm.exe -m 16384 e0_dr_lines.asm bin\e0_dr_lines.kex
@fasm.exe -m 16384 e1_scaling.asm bin\e1_scaling.kex
@fasm.exe -m 16384 e2_images.asm bin\e2_images.kex
@fasm.exe -m 16384 e3_text.asm bin\e3_text.kex
@fasm.exe -m 16384 e4_graf_ed.asm bin\e4_graf_ed.kex
@fasm.exe -m 16384 e5_lines_sm.asm bin\e5_lines_sm.kex

@kpack bin\e0_dr_lines.kex
@kpack bin\e1_scaling.kex
@kpack bin\e2_images.kex
@kpack bin\e3_text.kex
@kpack bin\e4_graf_ed.kex
@kpack bin\e5_lines_sm.kex
pause
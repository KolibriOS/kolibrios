if not exist bin mkdir bin
@fasm.exe -m 16384 ..\buf2d.asm bin\buf2d.obj
@kpack bin\buf2d.obj

if not exist bin\font8x9.bmp @copy ..\..\..\..\..\fs\kfar\trunk\font8x9.bmp bin\font8x9.bmp
@copy foto.jpg bin\foto.jpg
@copy img0.png bin\img0.png
@copy img1.png bin\img1.png
if not exist bin\vaz2106.vox @copy ..\..\..\..\..\media\voxel_editor\trunk\vaz2106.vox bin\vaz2106.vox

@fasm.exe -m 16384 e0_dr_lines.asm bin\e0_dr_lines.kex
@fasm.exe -m 16384 e1_scaling.asm bin\e1_scaling.kex
@fasm.exe -m 16384 e2_images.asm bin\e2_images.kex
@fasm.exe -m 16384 e3_text.asm bin\e3_text.kex
@fasm.exe -m 16384 e4_graf_ed.asm bin\e4_graf_ed.kex
@fasm.exe -m 16384 e5_lines_sm.asm bin\e5_lines_sm.kex
@fasm.exe -m 16384 e6_vox_1g.asm bin\e6_vox_1g.kex
@fasm.exe -m 16384 e7_vox_3g.asm bin\e7_vox_3g.kex
@fasm.exe -m 16384 e8_filters.asm bin\e8_filters.kex

@kpack bin\e0_dr_lines.kex
@kpack bin\e1_scaling.kex
@kpack bin\e2_images.kex
@kpack bin\e3_text.kex
@kpack bin\e4_graf_ed.kex
@kpack bin\e5_lines_sm.kex
@kpack bin\e6_vox_1g.kex
@kpack bin\e7_vox_3g.kex
@kpack bin\e8_filters.kex
pause
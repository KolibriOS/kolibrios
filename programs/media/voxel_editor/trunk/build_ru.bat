if not exist bin mkdir bin
if not exist bin\cursors.png @copy cursors.png bin\cursors.png
if not exist bin\cursors_gr.png @copy cursors_gr.png bin\cursors_gr.png
if not exist bin\toolbar.png @copy toolbar.png bin\toolbar.png
@copy *.vox bin\*.vox
if not exist bin\buf2d.obj @fasm.exe -m 16384 ..\..\..\develop\libraries\buf2d\trunk\buf2d.asm bin\buf2d.obj
@kpack bin\buf2d.obj
@fasm.exe -m 16384 voxel_editor.asm bin\voxel_editor.kex
@kpack bin\voxel_editor.kex
pause
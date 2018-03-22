if not exist bin mkdir bin
@copy *.vox bin\*.vox
if not exist bin\buf2d.obj @fasm.exe -m 16384 ..\..\..\develop\libraries\buf2d\trunk\buf2d.asm bin\buf2d.obj
@kpack bin\buf2d.obj
@fasm.exe -m 16384 voxel_editor.asm bin\voxel_editor.kex
@kpack bin\voxel_editor.kex
pause
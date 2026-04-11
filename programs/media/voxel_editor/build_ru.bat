if not exist bin mkdir bin
@copy *.vox bin\*.vox
@fasm.exe -m 16384 voxel_editor.asm bin\voxel_editor.kex
@kpack bin\voxel_editor.kex
pause
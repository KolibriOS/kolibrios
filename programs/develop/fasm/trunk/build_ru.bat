@fasm.exe -m 16384 ..\..\..\develop\libraries\box_lib\trunk\box_lib.asm box_lib.obj
@kpack box_lib.obj
@erase lang.inc
@echo lang fix ru >lang.inc
@fasm fasm.asm fasm
@erase lang.inc
@kpack fasm
@pause
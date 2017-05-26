if not exist bin mkdir bin
if not exist bin\msgbox.obj    @copy msgbox.obj bin\msgbox.obj
if not exist bin\info mkdir bin\info

if not exist bin\box_lib.obj @fasm.exe -m 16384 ..\..\develop\libraries\box_lib\trunk\box_lib.asm bin\box_lib.obj
@kpack bin\box_lib.obj

@erase lang.inc
@echo lang fix en >lang.inc
@fasm.exe -m 16384 t_edit.asm bin\t_edit.kex
@kpack bin\t_edit.kex

info\build.bat info\ bin\info\
pause
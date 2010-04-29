if not exist bin mkdir bin
if not exist bin\tl_sys_16.bmp @copy ..\..\develop\libraries\box_lib\trunk\tl_sys_16.bmp bin\tl_sys_16.bmp
if not exist bin\tl_nod_16.bmp @copy ..\..\develop\libraries\box_lib\trunk\tl_nod_16.bmp bin\tl_nod_16.bmp
if not exist bin\msgbox.obj    @copy msgbox.obj bin\msgbox.obj
if not exist bin\te_icon.bmp   @copy te_icon.bmp bin\te_icon.bmp
if not exist bin\info mkdir bin\info
copy info\* bin\info\*

if not exist bin\box_lib.obj @fasm.exe -m 16384 ..\..\develop\libraries\box_lib\trunk\box_lib.asm bin\box_lib.obj
@kpack bin\box_lib.obj

@erase lang.inc
@echo lang fix en >lang.inc
@fasm.exe -m 16384 t_edit.asm bin\t_edit.kex
@kpack bin\t_edit.kex
pause
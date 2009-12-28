if not exist bin mkdir bin
@fasm.exe  -m 16384 ..\..\develop\libraries\box_lib\trunk\box_lib.asm bin\box_lib.obj
@kpack bin\box_lib.obj

@fasm.exe  -m 16384 t_edit.asm bin\t_edit.kex
@kpack bin\t_edit.kex
@copy ..\..\develop\libraries\box_lib\trunk\tl_sys_16.bmp bin\tl_sys_16.bmp
@copy ..\..\develop\libraries\box_lib\trunk\tl_nod_16.bmp bin\tl_nod_16.bmp
@copy msgbox.obj bin\msgbox.obj
@copy te_icon.bmp bin\te_icon.bmp
if not exist bin\info mkdir bin\info
copy info\* bin\info\*
pause
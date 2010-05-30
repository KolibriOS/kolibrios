if not exist bin mkdir bin
if not exist bin\tl_sys_16.bmp @copy ..\..\develop\libraries\box_lib\trunk\tl_sys_16.bmp bin\tl_sys_16.bmp
if not exist bin\tl_nod_16.bmp @copy ..\..\develop\libraries\box_lib\trunk\tl_nod_16.bmp bin\tl_nod_16.bmp
if not exist bin\info mkdir bin\info
copy ..\t_edit\info\* bin\info\*

if not exist bin\box_lib.obj @fasm.exe -m 16384 ..\..\develop\libraries\box_lib\trunk\box_lib.asm bin\box_lib.obj
@kpack bin\box_lib.obj

@fasm.exe -m 16384 te_syntax.asm bin\te_syntax.kex
@kpack bin\te_syntax.kex
pause
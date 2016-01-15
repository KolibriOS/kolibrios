if not exist bin mkdir bin
@erase lang.inc
@echo lang fix en >lang.inc
@copy objects.png bin\objects.png
if not exist bin\info3ds.ini @copy info3ds.ini bin\info3ds.ini
if not exist bin\tl_sys_16.png @copy tl_sys_16.png bin\tl_sys_16.png
if not exist bin\toolbar.png @copy toolbar.png bin\toolbar.png
if not exist bin\font8x9.bmp @copy ..\..\fs\kfar\trunk\font8x9.bmp bin\font8x9.bmp
@fasm.exe -m 16384 ..\libraries\TinyGL\asm_fork\tinygl.asm bin\tinygl.obj
@kpack bin\tinygl.obj
@fasm.exe -m 16384 info3ds.asm bin\info3ds.kex
@kpack bin\info3ds.kex
@fasm.exe -m 16384 info3ds_u.asm bin\info3ds_u.kex
@kpack bin\info3ds_u.kex
pause
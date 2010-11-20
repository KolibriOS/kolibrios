if not exist bin mkdir bin
@fasm.exe -m 16384 kol_f_edit.asm bin\kol_f_edit.kex
@kpack bin\kol_f_edit.kex
@fasm.exe -m 16384 ob_o.asm bin\ob_o.opt

if not exist bin\tl_sys_16.png @copy tl_sys_16.png bin\tl_sys_16.png
if not exist bin\icon.bmp @copy icon.bmp bin\icon.bmp
if not exist bin\base.bmp @copy base.bmp bin\base.bmp
if not exist bin\left.bmp @copy left.bmp bin\left.bmp
if not exist bin\oper.bmp @copy oper.bmp bin\oper.bmp
if not exist bin\font6x9.bmp @copy ..\..\fs\kfar\trunk\font6x9.bmp bin\font6x9.bmp
if not exist bin\asm.syn @copy ..\..\other\t_edit\info\asm.syn bin\asm.syn

copy *.ced bin\*.ced
@fasm.exe -m 16384 ..\libraries\buf2d\trunk\buf2d.asm bin\buf2d.obj
@kpack bin\buf2d.obj
pause
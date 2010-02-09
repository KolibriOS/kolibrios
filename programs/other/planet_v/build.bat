if not exist bin mkdir bin
if not exist bin\tl_sys_16.png @copy tl_sys_16.png bin\tl_sys_16.png
if not exist bin\tl_nod_16.bmp @copy tl_nod_16.bmp bin\tl_nod_16.bmp
if not exist bin\planet_v.ini @copy planet_v.ini bin\planet_v.ini
if not exist bin\pl_metki.lst @copy pl_metki.lst bin\pl_metki.lst
if not exist bin\str.obj @copy str.obj bin\str.obj

@fasm.exe -m 16384 planet_v.asm bin\planet_v.kex
@kpack bin\planet_v.kex
pause
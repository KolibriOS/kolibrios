if not exist bin mkdir bin
if not exist bin\tl_sys_16.png @copy ..\..\media\log_el\trunk\tl_sys_16.png bin\tl_sys_16.png
if not exist bin\tl_nod_16.png @copy ..\t_edit\tl_nod_16.png bin\tl_nod_16.png

@fasm.exe -m 16384 te_syntax.asm bin\te_syntax.kex
@kpack bin\te_syntax.kex

pause
if not exist bin mkdir bin
if not exist bin\info mkdir bin\info

@erase lang.inc
@echo lang fix en >lang.inc
@fasm.exe -m 16384 t_edit.asm bin\t_edit.kex
@kpack bin\t_edit.kex

info\build.bat info\ bin\info\
pause
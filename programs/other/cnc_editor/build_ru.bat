if not exist bin mkdir bin
@copy *.nc bin\*.nc

@erase lang.inc
@echo lang fix ru >lang.inc

@fasm.exe -m 32768 cnc_editor.asm bin\cnc_editor.kex
@kpack bin\cnc_editor.kex
pause
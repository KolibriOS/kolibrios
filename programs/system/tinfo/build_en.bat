@echo lang fix en_EN >lang.inc
@fasm.exe -m 16384 tinfo.asm tinfo.kex
@kpack tinfo.kex
pause

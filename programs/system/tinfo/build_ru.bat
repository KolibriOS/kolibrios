@echo lang fix ru_RU >lang.inc
@fasm.exe -m 16384 tinfo.asm tinfo.kex
@kpack tinfo.kex
pause

@echo lang fix ru_RU >lang.inc
@fasm.exe -m 16384 c4.asm c4.kex
@kpack c4.kex
pause

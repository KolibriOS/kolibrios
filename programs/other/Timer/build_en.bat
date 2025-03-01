@echo lang fix en_EN >lang.inc
@fasm.exe -m 16384 timer.asm timer.kex
@kpack timer.kex
pause

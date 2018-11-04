@echo lang fix en >lang.inc
@fasm.exe -m 16384 life3.asm life3.kex
@kpack life3.kex
pause

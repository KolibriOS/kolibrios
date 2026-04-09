@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm -m 16384 cpu.asm cpu
@kpack cpu
@erase lang.inc
@pause

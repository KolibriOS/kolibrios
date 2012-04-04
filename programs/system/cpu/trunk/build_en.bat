@erase lang.inc
@echo lang fix en >lang.inc
@fasm -m 16384 cpu.asm cpu
@kpack cpu
@erase lang.inc
@pause
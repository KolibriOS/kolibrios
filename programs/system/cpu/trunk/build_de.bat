@erase lang.inc
@echo lang fix de_DE >lang.inc
@fasm -m 16384 cpu.asm cpu
@kpack cpu
@erase lang.inc
@pause

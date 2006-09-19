@erase lang.inc
@echo lang fix ge >lang.inc
@fasm kernel.asm kernel.mnt
@erase lang.inc
@pause
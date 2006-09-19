@erase lang.inc
@echo lang fix ru >lang.inc
@fasm kernel.asm kernel.mnt
@erase lang.inc
@pause
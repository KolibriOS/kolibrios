@erase lang.inc
@echo lang fix en >lang.inc
@fasm kernel.asm kernel.mnt
@erase lang.inc
@pause
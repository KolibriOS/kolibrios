@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm mblocks.asm mblocks
@erase lang.inc
@pause

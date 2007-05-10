@erase lang.inc
@echo lang fix en >lang.inc
@fasm netsendc.asm netsendc
@erase lang.inc
@pause
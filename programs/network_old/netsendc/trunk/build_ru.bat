@erase lang.inc
@echo lang fix ru >lang.inc
@fasm netsendc.asm netsendc
@erase lang.inc
@pause
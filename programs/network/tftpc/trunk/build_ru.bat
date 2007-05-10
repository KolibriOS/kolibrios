@erase lang.inc
@echo lang fix ru >lang.inc
@fasm tftpc.asm tftpc
@erase lang.inc
@pause
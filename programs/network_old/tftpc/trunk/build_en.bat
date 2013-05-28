@erase lang.inc
@echo lang fix en >lang.inc
@fasm tftpc.asm tftpc
@erase lang.inc
@pause
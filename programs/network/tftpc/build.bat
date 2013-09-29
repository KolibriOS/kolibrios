@erase lang.inc
@echo lang fix en >lang.inc
@fasm -m 16384 tftpc.asm tftpc
@kpack tftpc
@erase lang.inc
@pause
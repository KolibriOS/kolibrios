@erase lang.inc
@echo lang fix en >lang.inc
@fasm httpc.asm httpc
@erase lang.inc
@pause
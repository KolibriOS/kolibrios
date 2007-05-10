@erase lang.inc
@echo lang fix ru >lang.inc
@fasm httpc.asm httpc
@erase lang.inc
@pause
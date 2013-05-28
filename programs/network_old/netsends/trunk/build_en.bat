@erase lang.inc
@echo lang fix en >lang.inc
@fasm netsends.asm netsends
@erase lang.inc
@pause
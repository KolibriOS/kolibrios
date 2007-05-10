@erase lang.inc
@echo lang fix ru >lang.inc
@fasm netsends.asm netsends
@erase lang.inc
@pause
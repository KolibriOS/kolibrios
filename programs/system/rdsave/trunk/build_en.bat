@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm rdsave.asm rdsave
@kpack rdsave
@erase lang.inc
@pause

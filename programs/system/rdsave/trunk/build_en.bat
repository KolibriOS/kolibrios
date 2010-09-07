@erase lang.inc
@echo lang fix en >lang.inc
@fasm rdsave.asm rdsave
@kpack rdsave
@erase lang.inc
@pause
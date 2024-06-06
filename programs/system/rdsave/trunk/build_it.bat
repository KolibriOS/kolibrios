@erase lang.inc
@echo lang fix it_IT >lang.inc
@fasm rdsave.asm rdsave
@kpack rdsave
@erase lang.inc
@pause

@erase lang.inc
@echo lang fix ru_RU >lang.inc
@fasm rdsave.asm rdsave
@kpack rdsave
@erase lang.inc
@pause

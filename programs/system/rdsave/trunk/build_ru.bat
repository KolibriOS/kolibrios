@erase lang.inc
@echo lang fix ru >lang.inc
@fasm rdsave.asm rdsave
@kpack rdsave
@erase lang.inc
@pause
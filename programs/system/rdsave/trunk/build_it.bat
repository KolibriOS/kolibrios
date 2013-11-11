@erase lang.inc
@echo lang fix it >lang.inc
@fasm rdsave.asm rdsave
@kpack rdsave
@erase lang.inc
@pause
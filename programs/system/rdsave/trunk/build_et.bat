@erase lang.inc
@echo lang fix et >lang.inc
@fasm rdsave.asm rdsave
@kpack rdsave
@erase lang.inc
@pause
@echo lang fix en_US >lang.inc
@fasm cObj.asm cObj
@kpack cObj
@erase lang.inc
@pause

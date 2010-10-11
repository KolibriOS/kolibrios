@echo lang fix ru >lang.inc
@fasm cObj.asm cObj
@kpack cObj
@erase lang.inc
@pause
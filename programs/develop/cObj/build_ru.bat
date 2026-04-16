@echo lang fix ru_RU >lang.inc
@fasm cObj.asm cObj
@kpack cObj
@erase lang.inc
@pause

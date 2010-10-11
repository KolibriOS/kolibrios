@echo lang fix en >lang.inc
@fasm cObj.asm cObj
@kpack cObj
@erase lang.inc
@pause
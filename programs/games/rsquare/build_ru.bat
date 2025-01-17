@echo lang fix ru_RU >lang.inc
@fasm rsquare.asm rsquare
@erase lang.inc
kpack rsquare
@pause

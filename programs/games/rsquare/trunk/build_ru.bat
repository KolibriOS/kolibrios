@echo lang fix ru >lang.inc
@fasm rsquare.asm rsquare
@erase lang.inc
kpack rsquare
@pause
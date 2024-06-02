@echo lang fix ru_RU >lang.inc
@fasm disptest.asm disptest
@erase lang.inc
kpack disptest
@pause

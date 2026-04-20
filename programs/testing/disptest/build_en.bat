@echo lang fix en_US >lang.inc
@fasm disptest.asm disptest
@erase lang.inc
kpack disptest
@pause

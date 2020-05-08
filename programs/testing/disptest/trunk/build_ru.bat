@echo lang fix ru >lang.inc
@fasm disptest.asm disptest
@erase lang.inc
kpack disptest
@pause
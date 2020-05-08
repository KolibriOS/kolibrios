@echo lang fix en >lang.inc
@fasm disptest.asm disptest
@erase lang.inc
kpack disptest
@pause
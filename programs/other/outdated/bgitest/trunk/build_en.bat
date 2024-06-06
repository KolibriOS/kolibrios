@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm bgitest.asm bgitest
@erase lang.inc
@pause

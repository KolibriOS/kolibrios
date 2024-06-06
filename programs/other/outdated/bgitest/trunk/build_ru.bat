@erase lang.inc
@echo lang fix ru_RU >lang.inc
@fasm bgitest.asm bgitest
@erase lang.inc
@pause

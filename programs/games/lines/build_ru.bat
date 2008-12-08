@erase /s lang.inc > nul
@echo lang fix ru >lang.inc
@fasm lines.asm lines.kex
@erase lang.inc
@pause
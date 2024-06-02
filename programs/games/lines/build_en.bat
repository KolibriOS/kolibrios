@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm lines.asm lines.kex
@erase lang.inc
@pause

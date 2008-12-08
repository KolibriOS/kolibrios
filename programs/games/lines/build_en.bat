@erase lang.inc
@echo lang fix en >lang.inc
@fasm lines.asm lines.kex
@erase lang.inc
@pause
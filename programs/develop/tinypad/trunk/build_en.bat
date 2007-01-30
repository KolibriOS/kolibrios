@echo lang fix en >lang.inc
@fasm tinypad.asm tinypad
@erase lang.inc
@pause
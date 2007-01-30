@echo lang fix ru >lang.inc
@fasm tinypad.asm tinypad
@erase lang.inc
@pause
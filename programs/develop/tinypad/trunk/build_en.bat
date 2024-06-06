@echo lang fix en_US >lang.inc
@fasm tinypad.asm tinypad
@kpack tinypad
@erase lang.inc
@pause

@echo lang fix en >lang.inc
@fasm tinypad.asm tinypad
@kpack tinypad
@erase lang.inc
@pause
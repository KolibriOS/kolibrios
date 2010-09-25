@echo lang fix ru >lang.inc
@fasm tinypad.asm tinypad
@kpack tinypad
@erase lang.inc
@pause
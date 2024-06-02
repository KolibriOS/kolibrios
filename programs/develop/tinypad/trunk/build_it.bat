@echo lang fix it_IT >lang.inc
@fasm tinypad.asm tinypad
@kpack tinypad
@erase lang.inc
@pause

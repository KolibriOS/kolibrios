@echo lang fix ru_RU >lang.inc
@fasm tinypad.asm tinypad
@kpack tinypad
@erase lang.inc
@pause

@echo lang fix et >lang.inc
@fasm tinypad.asm tinypad
@kpack tinypad
@erase lang.inc
@pause
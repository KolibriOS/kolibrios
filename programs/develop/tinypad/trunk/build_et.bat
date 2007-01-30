@echo lang fix et >lang.inc
@fasm tinypad.asm tinypad
@erase lang.inc
@pause
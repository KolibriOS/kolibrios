@erase lang.inc
@echo lang fix et >lang.inc
@fasm tinypad.asm tinypad.bin
@erase lang.inc
@pause
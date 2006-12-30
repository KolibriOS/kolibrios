@erase lang.inc
@echo lang fix en >lang.inc
@fasm tinypad.asm tinypad.bin
@erase lang.inc
@pause
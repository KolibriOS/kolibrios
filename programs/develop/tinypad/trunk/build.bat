@echo lang fix en >lang.inc
@fasm -m 16384 tinypad.asm tinypad 
@erase lang.inc
@kpack tinypad
@pause

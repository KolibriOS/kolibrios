@erase lang.inc
@echo lang fix en >lang.inc
@fasm -m 16384 taskbar.asm panel
@erase lang.inc
@kpack panel
@pause
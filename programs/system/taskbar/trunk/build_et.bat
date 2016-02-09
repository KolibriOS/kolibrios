@erase lang.inc
@echo lang fix et >lang.inc
@fasm -m 16384 taskbar.asm @taskbar
@erase lang.inc
@kpack @taskbar
@pause
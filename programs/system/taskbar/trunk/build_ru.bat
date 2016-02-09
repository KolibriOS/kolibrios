@erase lang.inc
@echo lang fix ru >lang.inc
@fasm -m 16384 taskbar.asm @taskbar
@erase lang.inc
@kpack @taskbar
@pause
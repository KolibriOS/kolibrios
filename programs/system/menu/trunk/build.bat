@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm -m 16384 menu.asm @menu
@kpack @menu
@erase lang.inc
@pause

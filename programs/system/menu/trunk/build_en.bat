@erase lang.inc
@echo lang fix en >lang.inc
@fasm -m 16384 menu.asm @menu
@kpack @menu
@erase lang.inc
@pause
@echo lang fix en >lang.inc
@fasm -m 16384 @icon.asm @icon 
@erase lang.inc
@kpack @icon
@pause

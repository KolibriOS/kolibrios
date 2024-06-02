@echo lang fix en_US >lang.inc
@fasm -m 16384 icon.asm @icon 
@erase lang.inc
@kpack @icon
@pause

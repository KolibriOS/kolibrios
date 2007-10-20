@erase lang.inc
@echo lang fix ru >lang.inc
@fasm icon.asm @icon
@erase lang.inc
@kpack @icon
@pause
@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm open.asm @open
@kpack @open
@erase lang.inc
@pause

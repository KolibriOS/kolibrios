@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm bcdclk.asm bcdclk
@erase lang.inc
@pause

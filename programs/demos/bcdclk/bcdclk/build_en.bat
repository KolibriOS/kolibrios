@erase lang.inc
@echo lang fix en >lang.inc
@fasm bcdclk.asm bcdclk
@erase lang.inc
@pause
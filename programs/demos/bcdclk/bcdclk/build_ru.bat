@erase lang.inc
@echo lang fix ru >lang.inc
@fasm bcdclk.asm bcdclk
@erase lang.inc
@pause
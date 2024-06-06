@erase lang.inc
@echo lang fix ru_RU >lang.inc
@fasm bcdclk.asm bcdclk
@erase lang.inc
@pause

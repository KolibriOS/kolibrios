@erase lang.inc
@echo lang fix ru_RU >lang.inc
@fasm fspeed.asm fspeed
@kpack fspeed
@erase lang.inc
@pause

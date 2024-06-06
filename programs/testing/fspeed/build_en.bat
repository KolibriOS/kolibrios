@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm fspeed.asm fspeed
@kpack fspeed
@erase lang.inc
@pause

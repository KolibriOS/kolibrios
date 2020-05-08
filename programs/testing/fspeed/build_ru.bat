@erase lang.inc
@echo lang fix ru >lang.inc
@fasm fspeed.asm fspeed
@kpack fspeed
@erase lang.inc
@pause
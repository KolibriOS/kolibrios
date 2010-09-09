@erase lang.inc
@echo lang fix ru >lang.inc
@fasm desktop.asm desktop
@kpack desktop
@erase lang.inc
@pause
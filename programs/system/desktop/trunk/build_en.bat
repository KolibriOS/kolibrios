@erase lang.inc
@echo lang fix en >lang.inc
@fasm desktop.asm desktop
@kpack desktop
@erase lang.inc
@pause
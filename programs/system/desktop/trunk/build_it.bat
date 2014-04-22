@erase lang.inc
@echo lang fix it >lang.inc
@fasm desktop.asm desktop
@kpack desktop
@erase lang.inc
@pause
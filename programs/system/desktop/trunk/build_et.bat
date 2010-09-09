@erase lang.inc
@echo lang fix et >lang.inc
@fasm desktop.asm desktop
@kpack desktop
@erase lang.inc
@pause
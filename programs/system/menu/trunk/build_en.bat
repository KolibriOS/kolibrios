@erase lang.inc
@echo lang fix en >lang.inc
@fasm menu.asm @menu
@erase lang.inc
@pause
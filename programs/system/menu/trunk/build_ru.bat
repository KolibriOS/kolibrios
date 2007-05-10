@erase lang.inc
@echo lang fix ru >lang.inc
@fasm menu.asm @menu
@erase lang.inc
@pause
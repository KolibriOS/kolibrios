@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm color_dialog.asm colrdial
@erase lang.inc
@pause

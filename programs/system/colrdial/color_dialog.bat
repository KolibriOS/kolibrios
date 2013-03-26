@erase lang.inc
@echo lang fix en >lang.inc
@fasm color_dialog.asm colrdial
@kpack colrdial
@erase lang.inc
@pause
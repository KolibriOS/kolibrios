@erase lang.inc
@echo lang fix en >lang.inc
@fasm color_dialog.asm color_dialog
@kpack color_dialog
@erase lang.inc
@pause
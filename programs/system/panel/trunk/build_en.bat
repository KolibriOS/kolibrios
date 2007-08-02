@echo lang fix en >lang.inc
@fasm @panel.asm @panel
@erase lang.inc
@pause
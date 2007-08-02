@echo lang fix ru >lang.inc
@fasm @panel.asm @panel
@erase lang.inc
@pause
@echo lang fix en_US >lang.inc
@fasm piano.asm piano
@erase lang.inc
@pause

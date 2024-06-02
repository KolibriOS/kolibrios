@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm mixer.asm mixer
@erase lang.inc
@pause

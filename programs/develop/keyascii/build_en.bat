@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm keyascii.asm keyascii
@erase lang.inc
@pause

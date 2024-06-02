@erase lang.inc
@echo lang fix ru_RU >lang.inc
@fasm keyascii.asm keyascii
@erase lang.inc
@pause

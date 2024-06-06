@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm rtdata.asm rtdata
@erase lang.inc
@pause

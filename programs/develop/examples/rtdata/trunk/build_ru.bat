@erase lang.inc
@echo lang fix ru >lang.inc
@fasm rtdata.asm rtdata
@erase lang.inc
@pause
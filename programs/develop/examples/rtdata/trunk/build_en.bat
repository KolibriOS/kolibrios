@erase lang.inc
@echo lang fix en >lang.inc
@fasm rtdata.asm rtdata
@erase lang.inc
@pause
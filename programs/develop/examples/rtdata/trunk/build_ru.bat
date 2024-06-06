@erase lang.inc
@echo lang fix ru_RU >lang.inc
@fasm rtdata.asm rtdata
@erase lang.inc
@pause

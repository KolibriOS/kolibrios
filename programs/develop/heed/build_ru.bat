@erase lang.inc
@echo lang fix ru_RU >lang.inc
@fasm heed.asm heed
@kpack heed
@erase lang.inc
@pause

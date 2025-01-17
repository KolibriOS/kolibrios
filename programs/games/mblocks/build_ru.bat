@erase lang.inc
@echo lang fix ru_RU >lang.inc
@fasm mblocks.asm mblocks
@kpack mblocks
@erase lang.inc
@pause

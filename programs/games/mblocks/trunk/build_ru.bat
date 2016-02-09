@erase lang.inc
@echo lang fix ru >lang.inc
@fasm mblocks.asm mblocks
@kpack mblocks
@erase lang.inc
@pause
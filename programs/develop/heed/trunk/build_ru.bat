@erase lang.inc
@echo lang fix ru >lang.inc
@fasm heed.asm heed
@kpack heed
@erase lang.inc
@pause
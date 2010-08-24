@erase lang.inc
@echo lang fix en >lang.inc
@fasm heed.asm heed
@kpack heed
@erase lang.inc
@pause
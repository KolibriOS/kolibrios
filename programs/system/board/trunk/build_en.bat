@erase lang.inc
@echo lang fix en >lang.inc
@fasm board.asm board
@erase lang.inc
@pause
@erase lang.inc
@echo lang fix en >lang.inc
@fasm board.asm board
@kpack board
@erase lang.inc
@pause
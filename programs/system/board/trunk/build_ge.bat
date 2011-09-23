@erase lang.inc
@echo lang fix ge >lang.inc
@fasm board.asm board
@kpack board
@erase lang.inc
@pause
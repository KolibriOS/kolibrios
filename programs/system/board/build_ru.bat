@erase lang.inc
@echo lang fix ru_RU >lang.inc
@fasm board.asm board
@kpack board
@erase lang.inc
@pause

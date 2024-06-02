@erase lang.inc
@echo lang fix de_DE >lang.inc
@fasm board.asm board
@kpack board
@erase lang.inc
@pause

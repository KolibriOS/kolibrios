@erase lang.inc
@echo lang fix ru >lang.inc
@fasm board.asm board
@erase lang.inc
@pause
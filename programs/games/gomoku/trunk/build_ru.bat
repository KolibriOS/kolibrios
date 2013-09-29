@erase lang.inc
@echo lang fix ru >lang.inc
@fasm -m 16384 gomoku.asm gomoku
@kpack gomoku
@erase lang.inc
@pause
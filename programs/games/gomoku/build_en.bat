@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm -m 16384 gomoku.asm gomoku
@kpack gomoku
@erase lang.inc
@pause

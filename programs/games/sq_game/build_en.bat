@echo lang fix en_US >lang.inc
@fasm sq_game.asm sq_game
@erase lang.inc
@pause

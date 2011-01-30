@echo lang fix en >lang.inc
@fasm sq_game.asm sq_game
@erase lang.inc
@pause
@echo lang fix ru >lang.inc
@fasm sq_game.asm sq_game
@erase lang.inc
@pause
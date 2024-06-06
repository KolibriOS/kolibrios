@echo lang fix ru_RU >lang.inc
@fasm sq_game.asm sq_game
@erase lang.inc
@pause

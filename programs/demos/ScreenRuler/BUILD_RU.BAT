@erase lang.inc
@echo lang fix ru >lang.inc
@fasm ScreenRuler.asm ScreenRuler
@kpack ScreenRuler
@erase lang.inc
@pause
@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm ScreenRuler.asm ScreenRuler
@kpack ScreenRuler
@erase lang.inc
@pause

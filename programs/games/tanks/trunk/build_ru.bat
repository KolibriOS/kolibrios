@erase lang.inc
@echo lang fix ru_RU >lang.inc
@fasm tanks.asm tanks
@kpack tanks
@erase lang.inc
@pause

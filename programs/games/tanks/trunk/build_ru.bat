@erase lang.inc
@echo lang fix ru >lang.inc
@fasm tanks.asm tanks
@kpack tanks
@erase lang.inc
@pause
@erase lang.inc
@echo lang fix en >lang.inc
@fasm tanks.asm tanks
@kpack tanks
@erase lang.inc
@pause
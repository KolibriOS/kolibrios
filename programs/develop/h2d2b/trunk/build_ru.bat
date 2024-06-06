@erase lang.inc
@echo lang fix ru_RU >lang.inc
@fasm h2d2b.asm h2d2b
@kpack h2d2b
@erase lang.inc
@pause

@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm h2d2b.asm h2d2b
@kpack h2d2b
@erase lang.inc
@pause

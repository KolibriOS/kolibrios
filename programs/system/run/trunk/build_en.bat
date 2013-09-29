@erase lang.inc
@echo lang fix en >lang.inc
@fasm run.asm run
@kpack run
@erase lang.inc
@pause
@erase lang.inc
@echo lang fix en >lang.inc
@fasm run.asm run
@erase lang.inc
@pause
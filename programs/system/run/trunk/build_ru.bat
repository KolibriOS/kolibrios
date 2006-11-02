@erase lang.inc
@echo lang fix ru >lang.inc
@fasm run.asm run
@erase lang.inc
@pause
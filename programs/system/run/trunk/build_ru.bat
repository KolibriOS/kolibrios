@erase lang.inc
@echo lang fix ru >lang.inc
@fasm run.asm run
@kpack run
@erase lang.inc
@pause
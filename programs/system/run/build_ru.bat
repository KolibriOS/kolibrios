@erase lang.inc
@echo lang fix ru_RU >lang.inc
@fasm run.asm run
@kpack run
@erase lang.inc
@pause

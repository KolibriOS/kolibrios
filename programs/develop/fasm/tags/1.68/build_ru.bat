@erase lang.inc
@echo lang fix ru_RU >lang.inc
@fasm fasm.asm fasm
@erase lang.inc
@kpack fasm
@pause

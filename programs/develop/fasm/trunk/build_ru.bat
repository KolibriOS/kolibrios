@erase lang.inc
@echo lang fix ru >lang.inc
@fasm fasm.asm fasm
@erase lang.inc
@kpack fasm
@pause
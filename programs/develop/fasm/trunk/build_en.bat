@erase lang.inc
@echo lang fix en >lang.inc
@fasm -m 16384 fasm.asm fasm
@erase lang.inc
@kpack fasm
@pause
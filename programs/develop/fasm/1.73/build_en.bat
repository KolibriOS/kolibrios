@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm -m 16384 fasm.asm fasm
@erase lang.inc
@pause

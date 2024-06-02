@erase lang.inc
@echo lang fix ru_RU >lang.inc
@fasm -m 16384 fasm.asm fasm
@erase lang.inc
@pause

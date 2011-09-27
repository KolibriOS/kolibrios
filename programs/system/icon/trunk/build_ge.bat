@erase lang.inc
@echo lang fix ge >lang.inc
@fasm -m 16384 icon.asm icon
@kpack icon
@erase lang.inc
@pause
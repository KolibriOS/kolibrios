@erase lang.inc
@echo lang fix en >lang.inc
@fasm -m 16384 icon.asm icon
@kpack icon
@erase lang.inc
@pause
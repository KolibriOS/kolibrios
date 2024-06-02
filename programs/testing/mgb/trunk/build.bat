@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm -m 16384 mgb.asm mgb
@kpack mgb
@erase lang.inc
@pause

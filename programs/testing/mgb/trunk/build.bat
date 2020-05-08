@erase lang.inc
@echo lang fix en >lang.inc
@fasm -m 16384 mgb.asm mgb
@kpack mgb
@erase lang.inc
@pause
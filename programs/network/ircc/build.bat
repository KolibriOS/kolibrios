@erase lang.inc
@echo lang fix en >lang.inc
@fasm -m 16384 ircc.asm ircc
@kpack ircc
@erase lang.inc
@pause
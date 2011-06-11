@echo lang fix en >lang.inc
@fasm -m 16384 zSea.asm zSea
@erase lang.inc
@kpack zSea
@pause
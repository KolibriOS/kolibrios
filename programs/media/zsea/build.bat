@echo lang fix en_US >lang.inc
@fasm -m 16384 zSea.asm zSea
@erase lang.inc
@kpack zSea
@pause

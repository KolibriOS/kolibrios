@erase lang.inc
@echo lang fix ru_RU >lang.inc
@fasm trantest.asm trantest
@erase lang.inc
@pause

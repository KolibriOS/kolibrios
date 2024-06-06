@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm trantest.asm trantest
@erase lang.inc
@pause

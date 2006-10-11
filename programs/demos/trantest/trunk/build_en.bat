@erase lang.inc
@echo lang fix en >lang.inc
@fasm trantest.asm trantest
@erase lang.inc
@pause
@erase lang.inc
@echo lang fix ru >lang.inc
@fasm trantest.asm trantest
@erase lang.inc
@pause
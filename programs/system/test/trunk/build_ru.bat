@erase lang.inc
@echo lang fix ru >lang.inc
@fasm -m 16384 test.asm test
@kpack test
@erase lang.inc
@pause
@erase lang.inc
@echo lang fix ru_RU >lang.inc
@fasm -m 16384 test.asm test
@kpack test
@erase lang.inc
@pause

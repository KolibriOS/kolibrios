@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm -m 32768 docpack.asm docpack
@erase lang.inc
@kpack docpack
@pause

@echo lang fix et_EE >lang.inc
@fasm -m 16384 pipes.asm pipes
@erase lang.inc
@pause

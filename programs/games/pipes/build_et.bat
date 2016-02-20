@echo lang fix et >lang.inc
@fasm -m 16384 pipes.asm pipes
@erase lang.inc
@pause
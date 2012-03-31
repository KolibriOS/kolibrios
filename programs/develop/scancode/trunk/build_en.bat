@erase lang.inc
@echo lang fix en >lang.inc
@fasm -m 16384 scancode.asm scancode
@kpack scancode
@erase lang.inc
@pause
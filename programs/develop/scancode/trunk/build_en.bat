@erase lang.inc
@echo lang fix en >lang.inc
@fasm scancode.asm scancode
@erase lang.inc
@pause
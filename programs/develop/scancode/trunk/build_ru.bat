@erase lang.inc
@echo lang fix ru >lang.inc
@fasm scancode.asm scancode
@erase lang.inc
@pause
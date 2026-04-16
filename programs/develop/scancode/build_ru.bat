@erase lang.inc
@echo lang fix ru_RU >lang.inc
@fasm -m 16384 scancode.asm scancode
@kpack scancode
@erase lang.inc
@pause

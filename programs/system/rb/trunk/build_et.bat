@erase lang.inc
@echo lang fix et >lang.inc
@fasm -m 16384 @rb.asm @rb
@kpack @rb
@erase lang.inc
@pause
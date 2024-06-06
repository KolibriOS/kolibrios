@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm -m 16384 @rb.asm @rb
@kpack @rb
@erase lang.inc
@pause

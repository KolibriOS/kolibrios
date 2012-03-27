@erase lang.inc
@echo lang fix ru >lang.inc
@fasm @rb.asm @rb
@kpack @rb
@erase lang.inc
@pause
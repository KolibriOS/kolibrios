@erase lang.inc
@echo lang fix en >lang.inc
@fasm @rb.asm @rb
@kpack @rb
@erase lang.inc
@pause